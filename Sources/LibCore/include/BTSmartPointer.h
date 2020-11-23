#pragma once

namespace BT
{
    template <typename DataType>
    class SmartPointer final
    {
	    public:
	    //-------------------------------------------------------------------------------
	    //							Constructor/Destructor methods
	    //-------------------------------------------------------------------------------
		    SmartPointer():
		    m_pData(NULL), m_szCounter(0)
		    {}
	
		    SmartPointer(DataType* pValue):
		    m_pData(pValue), m_szCounter(1)
		    {}

		    SmartPointer(const SmartPointer<DataType>& sp):
		    m_pData(sp.m_pData), m_szCounter(sp.m_szCounter+1)
		    {}

		    virtual ~SmartPointer()
		    {
			    Release();
		    }

		    void Release()
		    {
			    m_szCounter--;
			    if(m_szCounter == 0)
			    {
				    delete m_pData;
			    }
			    m_pData=NULL;
		    }

	    //-------------------------------------------------------------------------------
	    //								Operator methods
	    //-------------------------------------------------------------------------------
		    DataType& operator*()
		    {
			    return *m_pData;
		    }
	
		    DataType* operator->()
		    {
			    return m_pData;
		    }
	
		    SmartPointer<DataType>& operator = (const SmartPointer<DataType>& sp)
		    {
			    //If is an other smart-pointer
			    if(this != &sp)
			    {
				    //Release this smart pointer
				    Release();

				    //And affect new value (+ update count)
				    m_pData		= sp.m_pData;
				    m_szCounter = m_szCounter++;
			    }
			    return *this;
		    }

		    bool operator< (const SmartPointer<DataType>& spCompare) const
		    {
			    return (m_pData < spCompare.m_pData);
		    }
        private:
            DataType*   m_pData;
            size_t	    m_szCounter;
    };
}