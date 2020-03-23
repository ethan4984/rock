#include <alloc.h>
#include <shitio.h>

using namespace standardout;

template<typename T>
class vector {
    public:
        vector()
        {
            array = (T*)malloc(sizeof(T));
            fsize = 1;
            iterator = 0;
        }

        vector(uint64_t size)
        {
            array = (T*)malloc(sizeof(T) * fsize);
            fsize = ((sizeof(T) * fsize) / sizeof(T));
            iterator = 0;
        }

        void operator =(T *initalizer)
        {
            uint64_t i = 0;
            while(initalizer[i++])
                push_back(initalizer[i]);
        }
        
        void erase(uint64_t index)
        {
            T *tmp = (T*)malloc(sizeof(T) * iterator - sizeof(T));
            
            int offset = 0;
            for(uint64_t i = 0; i < fsize; i++) {
                if(i == index)
                    offset++;
                tmp[i] = array[i + offset]; 
            }
            
            free(array);
            fsize -= 1;
            array = tmp;
            
            iterator--;
        }
        
        void push_back(T data)
        {
            if(iterator == fsize) {
                T *tmp = (T*)malloc((sizeof(T) * fsize) + sizeof(T));

                for(uint64_t i = 0; i < fsize; i++)
                    tmp[i] = array[i];

                free(array);
                fsize += 1;
                array = tmp;
            }

            array[iterator] = data;
            iterator++;
        }

        T at(uint64_t index)
        {
            if(iterator > index)
                return array[index];
            return 0;
        }

        uint64_t size()
        {
            return fsize;
        }
        
        uint64_t used()
        {
            return iterator;
        }
    private:
        T *array;
        uint64_t iterator;
        uint64_t fsize;
};
