##############################################################
#
# $ID$
#
# Makefile for sd-client
# Based on APIs provided in libsdv2
#
# Copyright 2002-2016 Network Appliance, Inc. All rights
# reserved. Specifications subject to change without notice.
#
# This SDK sample code is provided AS IS, with no support or
# warranties of any kind, including but not limited to
# warranties of merchantability or fitness of any kind,
# expressed or implied.  This code is subject to the license
# agreement that accompanies the SDK.
#
##############################################################

include ../../../Makefile.common

#CFLAGS += -DDEBUG
INCLOPTS += -I ../../../../libsdv2/include
LINKOPTS += -lsdv2

all: sdv2-client

sdv2-client: sdv2_client.c
	/usr/bin/gcc $(CFLAGS) -o sdv2-client ./sdv2_client.c $(INCLOPTS) $(LINKOPTS)
	@echo "Don't forget to set the correct LD_LIBRARY_PATH:"
	@echo "export LD_LIBRARY_PATH=$(realpath $(TOP))/sdk-staging/lib/linux-64:"'$$LD_LIBRARY_PATH'

clean:
	rm -f sdv2-client
