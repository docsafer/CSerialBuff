#include "StdAfx.h"
#include "CSerialBuff.h"
#include <assert.h>
 

CSerialBuff::CSerialBuff(void)
{
	m_pFields = NULL;
}

CSerialBuff::~CSerialBuff(void)
{
	Reset();
}

void CSerialBuff::Reset()
{
 
	struct tagFIELD* pField;
	while (pField = m_pFields)
	{
		m_pFields = pField->pNext;
        if (pField->fdt > FDT_DWORD)
        {
            if (pField->dwValueSize > 0 && pField->pValue)
            {
                HDelete(pField->pValue);
            }
        }
		HDelete(pField);
	}

}

 
struct tagFIELD& CSerialBuff::operator[](DWORD dwName)
{
	struct tagFIELD *pField = Get_Field(dwName, FDT_UNKNOWN);

	if (!pField)
	{
		pField = (struct tagFIELD*)HNew(sizeof(struct tagFIELD));
		
		pField->dwName = dwName;
		pField->fdt = FDT_UNKNOWN;
		pField->dwValueSize = 0;
		pField->pValue = NULL;
		pField->pNext = m_pFields;

		m_pFields = pField;
	}
 	return *pField;
}

BOOL CSerialBuff::Set_Field(DWORD dwName, BYTE *pValue, int nSize, FIELD_DATA_TYPE fdt)
{    

	if (!pValue || nSize < 0) return FALSE;


	struct tagFIELD *pField = Get_Field(dwName, fdt);


	if (pField == NULL)
	{
		pField = (struct tagFIELD*)HNew(sizeof(struct tagFIELD));
        if (pField == NULL) return FALSE;

		pField->pNext = m_pFields;
		m_pFields = pField;
	}
	else
	{
        if (pField->fdt > FDT_DWORD)
        {
            if (pField->pValue)
            {
                HDelete(pField->pValue);
                pField->pValue = NULL;
            }
        }
	}
 

    pField->dwName = dwName;
    pField->fdt = fdt;
    pField->dwValueSize = nSize;

    if (fdt > FDT_DWORD)
    {
        if (nSize)
        {
            pField->pValue = (LPBYTE)HNew(nSize);
            memcpy(pField->pValue, pValue, nSize);
        }
    }
    else
    {
        if (nSize >= sizeof(DWORD))
        {
            pField->dwValue = *(DWORD*)pValue;
        }
        else
        {
            pField->dwValue = 0;
        }
    }


    return TRUE;
}
int CSerialBuff::CalcSerializeSize()
{ 
	if (m_pFields == NULL) return 0;

	struct tagFIELD *pField = m_pFields;

	int nTotalSize = 0;
	do
	{
		nTotalSize += (sizeof(struct tagFIELD_INFO) + pField->dwValueSize); 
	}
	while(pField = pField->pNext);

	return nTotalSize;
}
int CSerialBuff::Serialize(PBYTE pData,int nDataSize)
{ 
	if (m_pFields == NULL) return 0;
 
	int nTotalSize=CalcSerializeSize();

	if(nDataSize<nTotalSize)
		return 0;
 
	struct tagFIELD *pField = m_pFields;

	struct tagFIELD_INFO *pInfo = (struct tagFIELD_INFO *)pData;

	do
	{
		pInfo->dwName = pField->dwName;
		pInfo->fdt = (FIELD_DATA_TYPE)pField->fdt;
		pInfo->dwValueSize = pField->dwValueSize;

		if (pField->fdt > FDT_DWORD)
		{
			if (pField->dwValueSize)
			{
				memcpy(pInfo+1, pField->pValue, pField->dwValueSize);
			}
		}
		else
		{
			assert(pField->dwValueSize == sizeof(DWORD));
			*(DWORD*)(pInfo+1) = pField->dwValue;
		}

		pInfo = (struct tagFIELD_INFO *)((PBYTE)(pInfo + 1) + pField->dwValueSize);
	}
	while(pField = pField->pNext);

	return nTotalSize;
}

int CSerialBuff::Serialize(PBYTE &pData)
{ 
	if (m_pFields == NULL) return 0;
 
	struct tagFIELD *pField = m_pFields;
 
	int nTotalSize = 0;
	do
	{
        nTotalSize += (sizeof(struct tagFIELD_INFO) + pField->dwValueSize); 
	}
    while(pField = pField->pNext);

    pData = (PBYTE)HNew(nTotalSize);
 
	pField = m_pFields;

    struct tagFIELD_INFO *pInfo = (struct tagFIELD_INFO *)pData;

	do
	{
        pInfo->dwName = pField->dwName;
        pInfo->fdt = (FIELD_DATA_TYPE)pField->fdt;
        pInfo->dwValueSize = pField->dwValueSize;

        if (pField->fdt > FDT_DWORD)
        {
            if (pField->dwValueSize)
            {
                memcpy(pInfo+1, pField->pValue, pField->dwValueSize);
            }
        }
        else
        {
            assert(pField->dwValueSize == sizeof(DWORD));
            *(DWORD*)(pInfo+1) = pField->dwValue;
        }

        pInfo = (struct tagFIELD_INFO *)((PBYTE)(pInfo + 1) + pField->dwValueSize);
	}
    while(pField = pField->pNext);
 
	return nTotalSize;
}

BOOL CSerialBuff::UnSerialize(PBYTE pData, int nTotalSize)
{ 
	 
	DWORD dwTemp=sizeof(struct tagFIELD_INFO);
	if (nTotalSize <= dwTemp)
		return FALSE;
 
	Reset();
 	 
	struct tagFIELD_INFO *pInfo = (struct tagFIELD_INFO *)pData;
    PBYTE pEnd = pData + nTotalSize;

	try
	{
		while (pInfo && ((PBYTE)(pInfo + 1) <= pEnd) && 
               ((PBYTE)(pInfo + 1) + pInfo->dwValueSize <= pEnd))
		{

            if (!Set_Field(pInfo->dwName, (PBYTE)(pInfo + 1), pInfo->dwValueSize, pInfo->fdt))
            { 
                break;
            }
            pInfo = (struct tagFIELD_INFO *)((PBYTE)(pInfo + 1) + pInfo->dwValueSize);
 
		}
	}
	catch (...)
	{
		Reset();
	}


	return m_pFields != NULL;
}

BOOL CSerialBuff::FieldExist(DWORD dwName, FIELD_DATA_TYPE fdt)
{
	if (!m_pFields) return FALSE;
	return Get_Field(dwName, fdt) != NULL;
}

BOOL CSerialBuff::GetFieldInfo(DWORD dwName, FIELD_DATA_TYPE fdt, struct tagFIELD_INFO &fi)
{
	if (!m_pFields) return FALSE;

	struct tagFIELD *pField = Get_Field(dwName, fdt);
	if (!pField) return FALSE;

    fi.dwName = pField->dwName;
    fi.fdt = (FIELD_DATA_TYPE)pField->fdt;
    fi.dwValueSize = pField->dwValueSize;
 
	return TRUE;
}

struct tagFIELD* CSerialBuff::Get_Field(DWORD dwName, FIELD_DATA_TYPE fdt)
{
	if (!m_pFields) return NULL;

    struct tagFIELD *pFound = NULL;
    struct tagFIELD *pField = m_pFields;
	do
	{
		if (dwName==pField->dwName && (fdt==pField->fdt || fdt==FDT_UNKNOWN))
		{ 	 
			pFound = pField;
			break;
		}
	}
    while(pField = pField->pNext);
 
	return pFound;
}

// ÉèÖÃÄ³¸ö×Ö¶ÎÖµ 
BOOL CSerialBuff::Set(DWORD dwName, int value)
{
     return Set_Field(dwName,(PBYTE)&value, sizeof(int), FDT_INT);
}

BOOL CSerialBuff:: Set(DWORD dwName, DWORD value)
{
    return Set_Field(dwName, (PBYTE)&value, sizeof(DWORD), FDT_DWORD);
}

BOOL CSerialBuff:: Set(DWORD dwName, double value)
{
    return Set_Field(dwName, (PBYTE)&value, sizeof(double), FDT_DOUBLE);
}

BOOL CSerialBuff:: Set(DWORD dwName, INT64 value)
{
    return Set_Field(dwName, (PBYTE)&value, sizeof(INT64), FDT_I64);
}

BOOL CSerialBuff:: Set(DWORD dwName, UINT64 value)
{
    return Set_Field(dwName, (PBYTE)&value, sizeof(UINT64), FDT_UI64);
}

BOOL CSerialBuff:: Set(DWORD dwName, LPCTSTR value)
{
	int nSize = (_tcslen(value) + 1) * sizeof(TCHAR);

    return Set_Field(dwName, (PBYTE)value, nSize, FDT_STRING);
}

BOOL CSerialBuff:: Set(DWORD dwName, PBYTE value, int size)
{
	return Set_Field(dwName, value, size, FDT_PBYTE);
}

 
BOOL CSerialBuff::Get(DWORD dwName, int& value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_INT);
	if(!pField) return FALSE;

	assert(pField->dwValueSize == sizeof(int));

	value = (int)pField->dwValue;

	return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, DWORD& value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_DWORD);
    if(!pField) return FALSE;

    assert(pField->dwValueSize == sizeof(DWORD));

    value = pField->dwValue;

    return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, double& value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_DOUBLE);
    if (!pField) return FALSE;

    assert(pField->dwValueSize == sizeof(double));

    value = *((double*)pField->pValue);

    return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, INT64& value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_I64);
    if (!pField) return FALSE;

    assert(pField->dwValueSize == sizeof(INT64));

    value = *((INT64*)pField->pValue);

    return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, UINT64& value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_UI64);
    if (!pField) return FALSE;

    assert(pField->dwValueSize == sizeof(UINT64));

    value = *((UINT64*)pField->pValue);

    return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, TCHAR* value)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_STRING);
    if (!pField) return FALSE;

    assert(pField->pValue);

    _tcscpy(value, (TCHAR*)pField->pValue);

    return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, wstring& value)
{
	struct tagFIELD *pField = Get_Field(dwName, FDT_STRING);
	if (!pField) return FALSE;

	assert(pField->pValue);

	value = (LPCTSTR)pField->pValue;

	return TRUE;
}

BOOL CSerialBuff::Get(DWORD dwName, PBYTE &value, int &size)
{
	struct tagFIELD *pField = Get_Field(dwName, FDT_PBYTE);
	if (!pField) return FALSE;

	assert(pField->pValue);

	size = pField->dwValueSize;
    if (size && pField->pValue)
    {
        value = (PBYTE)HNew(size);
        if (value == NULL) return FALSE;
        memcpy(value, pField->pValue, size);
    }

	return size>0;
}

BOOL CSerialBuff::GetValue(DWORD dwName, PBYTE value, int size)
{
    struct tagFIELD *pField = Get_Field(dwName, FDT_PBYTE);
    if (!pField) return FALSE;

    assert(pField->pValue);

	if (size != pField->dwValueSize) return FALSE;
    memcpy(value, pField->pValue, size);

	return TRUE;
}

BOOL CSerialBuff::Remove_Field(DWORD dwName, FIELD_DATA_TYPE fdt)
{ 
    if (!m_pFields) return FALSE;

    struct tagFIELD *pField = m_pFields;
	struct tagFIELD **ppNext = &m_pFields;

	BOOL bFind = FALSE;
	do
	{  
		if (dwName==pField->dwName && (fdt==pField->fdt || fdt==FDT_UNKNOWN))
		{
			if (pField->pValue)
			{
 			    HDelete(pField->pValue);
				pField->pValue = NULL;
			}

            *ppNext = pField->pNext;
  
			HDelete(pField);

			return TRUE;
 		}

		ppNext = &pField->pNext;
	}
    while(pField = pField->pNext);
 
    return FALSE;
}