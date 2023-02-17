############################################################################
# apps/system/easylogger/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

# easylogger Application

CSRCS  = elog_port.c elog.c elog_utils.c
CSRCS += elog_async.c elog_buf.c

CFLAGS += ${shell $(INCDIR) "$(CC)" $(APPDIR)/system/easylogger/inc}

VPATH += :src port

ifeq ($(CONFIG_SYSTEM_EASYLOGGER_FILE),y)
CSRCS += elog_file.c elog_file_port.c
CFLAGS += ${shell $(INCDIR) "$(CC)" $(APPDIR)/system/easylogger/plugins/file}
VPATH += :plugins/file
endif

ifeq ($(CONFIG_SYSTEM_EASYLOGGER_FLASH),y)
CSRCS += elog_flash.c elog_flash_port.c
CFLAGS += ${shell $(INCDIR) "$(CC)" $(APPDIR)/system/easylogger/plugins/flash}
VPATH += :plugins/flash
CFLAGS += ${shell $(INCDIR) "$(CC)" $(APPDIR)/system/easyflash/inc}
endif

include $(APPDIR)/Application.mk
