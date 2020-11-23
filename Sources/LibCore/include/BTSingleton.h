#pragma once

//Memory Leak Linker
#ifndef NDEBUG
	#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_NEW
#endif

namespace BT
{
    template <typename DataType>
    class Singleton
    {
        public:
            static DataType* get()
            {
                DataType* tmp = _instance.load();
                if(tmp == nullptr)
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    tmp = _instance.load();
                    if(tmp == nullptr)
                    {
                        tmp = new T;
                        _instance.store(tmp);
                    }
                }
                return tmp;
            }

            static void releaseSingleton()
            {
                DataType* tmp = _instance.load();
                if(tmp == nullptr)
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    tmp = _instance.load();
                    if(tmp == nullptr)
                    {
                        delete tmp;
                        tmp = NULL;
                        _instance.store(tmp);
                    }
                }
            }

        private:
            static std::atomic<DataType*>	_instance;
            static std::mutex		        _mutex;

        protected:
            Singleton() = default;
            virtual ~Singleton() = default;
    };
//Memory Leak Linker
#ifndef NDEBUG
#   undef DEBUG_NEW
#   undef new	
#endif

    //Initialization of singleton
    template <typename DataType>
    std::atomic<DataType*> Singleton<DataType>::_instance = NULL;

    template <typename DataType>
    std::mutex Singleton<DataType>::_mutex;
}