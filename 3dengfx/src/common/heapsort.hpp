/*
Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Heapsort template functions
 * 
 * Author: Mihalis Georgoulopoulos 2005
 */

#ifndef __HEAPSORT_HEADER
#define __HEAPSORT_HEADER

#define CHILD1(x) (x << 1)
#define CHILD2(x) ((x << 1) + 1)
#define PARENT(x) (x >> 1)

#define SORT_LOHI	false
#define SORT_HILO	true

template <class T> int less(const T &a, const T &b)
{
	return (a < b);
}

template <class T> int greater(const T &a, const T &b)
{
	return !(a < b);
}

template <class T> void sort(T *elements, int n, bool hilo)
{
	int (*less_than)(const T&, const T&);
	int (*greater_than)(const T&, const T&);

	if (hilo)
	{
		less_than = greater;
		greater_than = less;
	}
	else
	{
		less_than = less;
		greater_than = greater;
	}

	T *elements_mo = elements - 1;
	T swap;
	int pos, parent_pos, c1, c2, child;

	// --- heap construction ---
	for (int new_element=2; new_element<=n; new_element++)
	{
		pos = new_element;
		// restore heap condition
		while (pos > 1)
		{
			parent_pos = PARENT(pos);
			// swap with the parent , if in higher priority
			if (greater_than(elements_mo[pos], elements_mo[parent_pos]))
			{
				swap = elements_mo[parent_pos];
				elements_mo[parent_pos] = elements_mo[pos];
				elements_mo[pos] = swap;

				pos = parent_pos;
			}
			else
			{
				// this element is in its correct position
				break;
			}
		} // end restoring heap condition for new element
	} // end heap construction

	// --- heap deconstruction ---
	for (int removed_element=n; removed_element>1; removed_element--)
	{
		swap = elements_mo[1];
		elements_mo[1] = elements_mo[removed_element];
		elements_mo[removed_element] = swap;

		pos = 1;
		// now we have to restore the heap condition again
		while (pos < removed_element)
		{
			// decide which of the children is
			// in higher priority
			c1 = CHILD1(pos);
			c2 = CHILD2(pos);
			child = c1;
			if (c1 >= removed_element)
			{
				// finished
				break;
			}
			if ( c2 < removed_element && less_than(elements_mo[c1], elements_mo[c2]))
					child = c2;

			if (less_than(elements_mo[pos], elements_mo[child]))
			{
				swap = elements_mo[pos];
				elements_mo[pos] = elements_mo[child];
				elements_mo[child] = swap;

				pos = child;
			}
			else
			{
				// this element is in its correct position
				break;
			}
		} // end restoring heap condition
	} // end heap deconstruction
}

template <class T, class P> void sort(T *elements, P *priorities, int n, bool hilo)
{
	int (*less_than)(const P&, const P&);
	int (*greater_than)(const P&, const P&);

	if (hilo)
	{
		less_than = greater;
		greater_than = less;
	}
	else
	{
		less_than = less;
		greater_than = greater;
	}


	T *elements_mo = elements - 1;
	P *priorities_mo = priorities - 1;
	T swap_t;
	P swap_p;
	int pos, parent_pos, c1, c2, child;

	// --- heap construction ---
	for (int new_element=2; new_element<=n; new_element++)
	{
		pos = new_element;
		// restore heap condition
		while (pos > 1)
		{
			parent_pos = PARENT(pos);
			// swap with the parent , if in higher priority
			if (greater_than(priorities_mo[pos], priorities_mo[parent_pos]))
			{
				swap_t = elements_mo[parent_pos];
				elements_mo[parent_pos] = elements_mo[pos];
				elements_mo[pos] = swap_t;
				swap_p = priorities_mo[parent_pos];
				priorities_mo[parent_pos] = priorities_mo[pos];
				priorities_mo[pos] = swap_p;

				pos = parent_pos;
			}
			else
			{
				// this element is in its correct position
				break;
			}
		} // end restoring heap condition for new element
	} // end heap construction

	// --- heap deconstruction ---
	for (int removed_element=n; removed_element>1; removed_element--)
	{
		swap_t = elements_mo[1];
		elements_mo[1] = elements_mo[removed_element];
		elements_mo[removed_element] = swap_t;
		swap_p = priorities_mo[1];
		priorities_mo[1] = priorities_mo[removed_element];
		priorities_mo[removed_element] = swap_p;

		pos = 1;
		// now we have to restore the heap condition again
		while (pos < removed_element)
		{
			// decide which of the children is
			// in higher priority
			c1 = CHILD1(pos);
			c2 = CHILD2(pos);
			child = c1;
			if (c1 >= removed_element)
			{
				// finished
				break;
			}
			if ( c2 < removed_element && less_than(priorities_mo[c1], priorities_mo[c2]))
					child = c2;

			if (less_than(priorities_mo[pos], priorities_mo[child]))
			{
				swap_t = elements_mo[pos];
				elements_mo[pos] = elements_mo[child];
				elements_mo[child] = swap_t;
				swap_p = priorities_mo[pos];
				priorities_mo[pos] = priorities_mo[child];
				priorities_mo[child] = swap_p;

				pos = child;
			}
			else
			{
				// this element is in its correct position
				break;
			}
		} // end restoring heap condition
	} // end heap deconstruction
}

#endif // ndef __HEAPSORT_HEADER
