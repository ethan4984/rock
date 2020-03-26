#include <alloc.h>
#include <shitio.h>

using namespace standardout;

template<typename T>
class vector {
    public:
        vector()
        {
            t_print("bruh");
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

        vector(T *bruh)
        {
            uint64_t len = 0;
            while(bruh[len])
                len++;
            array = (T*)malloc(sizeof(T) * len);

            for(int i = 0; i < len; i++)
                array[i] = bruh[i];
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

            t_print("%x", array);

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

				t_print("ok %x", array);

                free(array);
                fsize += 1;
                array = tmp;
            }

            t_print("PUSHING");

            array[iterator] = data;
            iterator++;
        }

        void new_back()
        {
            if(iterator == fsize) {
                T *tmp = (T*)malloc((sizeof(T) * fsize) + sizeof(T));

                for(uint64_t i = 0; i < fsize; i++)
                    tmp[i] = array[i];

                free(array);
                fsize += 1;
                array = tmp;
            }
        }

        T at(uint64_t index)
        {
            if(iterator > index)
                return array[index];
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
