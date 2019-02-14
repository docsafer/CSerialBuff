#pragma once

#include <string>
#include <TCHAR.h>
 
using namespace std;
 
#define REQ_MAX_VALUE_SIZE  0xFFFFFF
 
 
#define HNew(size)     HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(size))
#define HDelete(p)     HeapFree(GetProcessHeap(),0,(p))


enum FIELD_DATA_TYPE
{
	FDT_UNKNOWN    = 0,
	FDT_INT        = 1,
	FDT_DWORD      = 2,
	FDT_DOUBLE     = 3,
	FDT_I64        = 4,
	FDT_STRING     = 5,
	FDT_PBYTE      = 6,
    FDT_UI64       = 7,
};

struct tagFIELD_INFO
{
	DWORD dwName;
    FIELD_DATA_TYPE fdt;
    DWORD dwValueSize;
};

struct tagFIELD
{
    DWORD            dwName;
    DWORD            fdt : 8;
    DWORD            dwValueSize : 24;
    union {
        DWORD        dwValue;
        PBYTE        pValue;
    };
    struct tagFIELD *pNext;

	struct tagFIELD()
	{   
		 memset(this,0,sizeof(struct tagFIELD));
 	};

    operator int()
    {
        if (fdt == FDT_INT)
        {
            return (int)dwValue;
        }
        return 0;
    }

	operator DWORD()
	{
		if (fdt == FDT_DWORD)
		{
			return dwValue;
		}
		return 0;
	}

	operator double()
	{
		if (fdt == FDT_DOUBLE && pValue)
		{
			return *(double*)pValue;
		}
		return 0;
	}

	operator INT64()
	{
		if (fdt == FDT_I64 && pValue)
		{
			return *(INT64*)pValue;
		}
		return 0;
	}
 
    operator UINT64()
    {
        if (fdt == FDT_UI64 && pValue)
        {
            return *(UINT64*)pValue;
        }
        return 0;
    }

	operator LPSTR()
	{
		if (fdt == FDT_STRING && pValue)
		{
			return (LPSTR)pValue;
		}
		return (LPSTR)NULL;
	}

	operator LPWSTR()
	{
		if (fdt == FDT_STRING && pValue)
		{
			return (LPWSTR)pValue;
		}
		return (LPWSTR)NULL;
	}

	operator PBYTE()
	{
		if (fdt == FDT_PBYTE && pValue)
		{
			return pValue;
		}
		return (PBYTE)NULL;
	}

    void operator=(int value)
    {
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
        fdt = FDT_INT;
        dwValueSize = sizeof(int);
        dwValue = value;
    }

	void operator=(DWORD value)
	{
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
		fdt = FDT_DWORD;
		dwValueSize = sizeof(DWORD);
		dwValue = value;
	}

	void operator=(double value)
	{
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
		fdt = FDT_DOUBLE;
		dwValueSize = sizeof(double);
		pValue = (PBYTE)HNew(sizeof(double));
		*(double*)pValue = value;
    }

	void operator=(INT64 value)
	{
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
		fdt = FDT_I64;
		dwValueSize = sizeof(INT64);
		pValue = (PBYTE)HNew(sizeof(INT64));
		*(INT64*)pValue = value;
	}

    void operator=(UINT64 value)
    {
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
        fdt = FDT_UI64;
        dwValueSize = sizeof(UINT64);
        pValue = (PBYTE)HNew(sizeof(UINT64));
        *(UINT64*)pValue = value;
    }
 
    void operator=(LPCSTR value)
    {
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
        fdt = FDT_STRING;
        if (value)
        {
            dwValueSize = (strlen(value)+1)*sizeof(CHAR);
            pValue = (PBYTE)HNew(dwValueSize);
            memcpy(pValue, value, dwValueSize);
        }
        else
        {
            dwValueSize = 0;
            pValue = NULL;
        }
    }

	void operator=(LPCWSTR value)
	{
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
		fdt = FDT_STRING;
        if (value)
        {
            dwValueSize = (wcslen(value)+1)*sizeof(WCHAR);
            pValue = (PBYTE)HNew(dwValueSize);
            memcpy(pValue, value, dwValueSize);
        }
        else
        {
            dwValueSize = 0;
            pValue = NULL;
        }
	}

	void operator()(PBYTE value, int nSize)
	{
        if (fdt > FDT_DWORD)
        {
            if (pValue) HDelete(pValue);
        }
		if (value && nSize > 0)
		{
			fdt = FDT_PBYTE;
			dwValueSize = nSize;
			pValue = (PBYTE)HNew(nSize);
			memcpy(pValue, value, nSize);

		}
		else
		{
			fdt = FDT_PBYTE;
			dwValueSize = 0;
			pValue = NULL;
		}
	}

	int operator()(PBYTE& value)
	{
		if (dwValueSize > 0 && pValue)
		{
			value = new BYTE[dwValueSize];
            if (fdt > FDT_DWORD)
            {
                memcpy(value, pValue, dwValueSize);
            }
            else
            {
                *(DWORD*)value = dwValue;
            }
            return dwValueSize;
		}
		value = NULL;
		return 0;
	}
};

 

class CSerialBuff
{
public:
	CSerialBuff(void);
	~CSerialBuff(void);

	BOOL UnSerialize(PBYTE pData, int size);
	int  Serialize(PBYTE &pData);
  
	 
	struct tagFIELD& operator[](DWORD dwName);
 
	 
	BOOL Set(DWORD dwName, int value);
    BOOL Set(DWORD dwName, DWORD value);

    BOOL Set(DWORD dwName, double value);
    BOOL Set(DWORD dwName, INT64 value);
    BOOL Set(DWORD dwName, UINT64 value);

	BOOL Set(DWORD dwName, LPCTSTR value);
	BOOL Set(DWORD dwName, PBYTE value, int size);

    
    BOOL Get(DWORD dwName, int& value);
    BOOL Get(DWORD dwName, DWORD& value);

    BOOL Get(DWORD dwName, double& value);
    BOOL Get(DWORD dwName, INT64& value);
    BOOL Get(DWORD dwName, UINT64& value);

	BOOL Get(DWORD dwName, TCHAR* value);
	BOOL Get(DWORD dwName, wstring& value);
	 
	BOOL Get(DWORD dwName, PBYTE &value, int &size);
	BOOL GetValue(DWORD dwName, PBYTE value, int size);
 	
	BOOL Remove_Field(DWORD dwName, FIELD_DATA_TYPE fdt);
    void Reset();
	BOOL FieldExist(DWORD dwName, FIELD_DATA_TYPE fdt);
	BOOL GetFieldInfo(DWORD dwName, FIELD_DATA_TYPE fdt, struct tagFIELD_INFO &fi);

private:
    BOOL Set_Field(DWORD dwName, BYTE *pValue, int nSize, FIELD_DATA_TYPE fdt);
	struct tagFIELD* Get_Field(DWORD dwName, FIELD_DATA_TYPE fdt);

private:
	struct tagFIELD  *m_pFields;
public:
	int  CalcSerializeSize();
	int  Serialize(PBYTE pData,int nDataSize);

};
