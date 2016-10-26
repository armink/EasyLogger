#ifndef __COLORS_H__
#define __COLORS_H__

/* use Escape Sequence control the text color
 * details please reference http://www.cnblogs.com/clover-toeic/p/4031618.html
 */

#define ESC_START       "\e["
#define ESC_END         "\e[0m"

//front color
#define F_BLACK   "30;"
#define F_RED     "31;"
#define F_GREEN   "32;"
#define F_YELLOW  "33;"
#define F_BLUE    "34;"
#define F_MAGENTA "35;"
#define F_CYAN    "36;"
#define F_WHITE   "37;"

//background color
#define B_BLACK   "40;"
#define B_RED     "41;"
#define B_GREEN   "42;"
#define B_YELLOW  "43;"
#define B_BLUE    "44;"
#define B_MAGENTA "45;"
#define B_CYAN    "46;"
#define B_WHITE   "47;"

//show style
#define NORMAL    "0m"
#define BOLD      "1m"
#define BLINK     "5m"
#define NO_BOLD   "22m"

//[front color] + [background color] + [show style]
#define COLOR_ASSERT    F_MAGENTA B_BLACK NO_BOLD
#define COLOR_ERROR     F_RED B_BLACK NO_BOLD
#define COLOR_WARN      F_YELLOW B_BLACK NO_BOLD
#define COLOR_INFO      F_BLUE B_BLACK NO_BOLD
#define COLOR_DEBUG     F_GREEN B_BLACK NO_BOLD
#define COLOR_VERBOSE   F_WHITE B_BLACK NO_BOLD

#endif
