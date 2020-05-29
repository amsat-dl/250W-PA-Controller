/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
  ******************************************************************************
  * @file    main.c
  * @author  Kurt Moraw, DJ0ABR
  * @version V1.0
  * @date    22.Nov 2019
  * @brief   Default main function.
  ******************************************************************************
*/


#include <main.h>


int main(void)
{
	// init hardware
	init_gpio();
	init_timer();
	init_uart();
	init_adc();
	init_display();
	display_greeting();

	while(1)
	{
		controller();
	}
}
