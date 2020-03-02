#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <port.h>
#include <paging.h>

/* colour constants */

#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_REEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GREY 7
#define VGA_DARK_GREY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN 14
#define VGA_WHITE 15

size_t strlen(const char *str);

int strcmp(const char *a, const char *b);

char *strcpy(char *dest, const char *src);

namespace standardout
{
	static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;

	void initalize(uint8_t bg, uint8_t fg);

	void k_print(const char str[256],...);

	void t_print(const char str[256],...);

	void putchar(char c);

	void clear_screen();

	bool end_of_terminal();

	bool end_of_screen(size_t offset);

	bool terminal_setcolor(uint8_t background, uint8_t text);
}

template<typename T>
class darry
{
	public:
		darry(const T *base, int current_size) : dynamic_array(base), arr_size(current_size)
        	{
			status = arr_size;
		}

		darry(int max_size) : arr_size(max_size)
        	{
			dynamic_array = new T[arr_size];
			status = 0;
		}

		darry()
        	{
			arr_size = 1;
			status = 0;
			dynamic_array = new T[arr_size];
		};

		~darry()
        	{
			delete[] dynamic_array;
		}

		void add(T new_element)
        	{
			if(status == arr_size) {
				T *tmp = new T[2 * arr_size];

				for (int i = 0; i < arr_size; i++)
					tmp[i] = dynamic_array[i];

				delete[] dynamic_array;
				arr_size *= 2;
				dynamic_array = tmp;
				delete[] tmp;
			}
			dynamic_array[status] = new_element;
			status++;
		}

		void del()
        	{
			status--;
		}

		void in_add(T new_element, int location)
		{
			T *tmp = new T[arr_size];

			for(int i = 0; i < arr_size; i++) {
				if(i == location - 1)
					tmp[i] = new_element;
					else
					tmp[i] = dynamic_array[i];
			}

			delete[] dynamic_array;
			dynamic_array = tmp;
			delete[] tmp;
		}

		void in_del(int location)
        	{
			T *tmp = new T[arr_size - 1];

			for(int i = 0; i < location - 1; i++)
				tmp[i] = dynamic_array[i];

			for(int i = location + 1; i < arr_size - 1; i++)
				tmp[i-1] = dynamic_array[i];

			delete[] dynamic_array;
			arr_size -= 1;
			dynamic_array = tmp;
			delete[] tmp;
		}

		void resize(int new_size)
        	{
			T *tmp = new T[new_size];

			for(int i = 0; i < new_size; i++)
				tmp[i] = dynamic_array[i];

			delete[] dynamic_array;
			arr_size = new_size;
			dynamic_array = tmp;
			delete[] tmp;
		}

		void clear()
        	{
			memset(dynamic_array, 0, size);
		}

		int size()
        	{
			return arr_size;
		}

		T grab(int index)
        	{
			return dynamic_array[index];
		}

		void print()
        	{
			for(int i = 0; i < arr_size; i++)
			k_print("%d", dynamic_array[i]);
		}

		void *operator new(size_t size)
        	{
      			return MM::malloc(size);
		}

		void operator delete(void *location)
        	{
			MM::free(location);
		}

		void *operator new[](unsigned long size)
        	{
       	    return MM::malloc(size);
    		}

		void operator delete[](void *location)
        	{
			MM::free(location);
		}

	private:
		int arr_size;

		int status;

		T  *dynamic_array;
};
