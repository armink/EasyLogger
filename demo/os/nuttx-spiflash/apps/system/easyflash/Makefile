############################################################################
# apps/system/easyflash/Makefile
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

# easyflash Application

CSRCS  = easyflash.c ef_utils.c ef_port.c
CSRCS += ef_env.c # ef_env_wl.c
# CSRCS += ef_iap.c ef_log.c

CFLAGS += ${shell $(INCDIR) "$(CC)" $(APPDIR)/system/easyflash/inc}

VPATH += :src port

include $(APPDIR)/Application.mk
