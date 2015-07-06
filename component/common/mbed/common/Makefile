
include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

MODULE_IFLAGS = -I../

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = 
OBJS_ROM = 
OBJS_RAM =
ifeq ($(CONFIG_TIMER_MODULE),y)
	OBJS += us_ticker_api.o wait_api.o
endif

ifeq ($(CONFIG_LIB_BUILD_RAM),y)
	OBJS = $(OBJS_RAM)
else ifeq ($(CONFIG_RELEASE_BUILD_RAM_ALL),y)
	OBJS += $(OBJS_RAM)
else ifeq ($(CONFIG_RELEASE_BUILD_LIBRARIES),y)
	OBJS = $(CSRC_ROM)
else ifeq ($(CONFIG_NORMAL_BUILD),y)
	OBJS += $(CSRC_ROM)
	OBJS += $(CSRC_RAM)
endif


#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: CORE_TARGETS COPY_RAM_OBJS
	
#*****************************************************************************#
#              GENERATE OBJECT FILE 
#*****************************************************************************#
CORE_TARGETS:	$(OBJS)


#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean:
	$(REMOVE) *.o
	$(REMOVE) *.i
	$(REMOVE) *.s
	$(REMOVE) *.d

-include $(DEPS)