.PHONY: all clean
CC= gcc
CFLAGS = -Wall -g -Wextra -Werror -O3 -Wpedantic -lpthread 
OUTPUT_OPTION= -MMD -MP -o $@
SOURCE= sched_demo_312512032.c
OBJS= $(SOURCE:.c=.o)
DEPS= $(SOURCE:.c=.d)

TARGET= sched_demo_312512032



all : $(TARGET)
-include $(DEPS)
$(TARGET): $(OBJS)
	$(CC) $^ -o $@   
	
	
clean :
	@rm -f $(TARGET) $(OBJS)  $(DEPS)
	
