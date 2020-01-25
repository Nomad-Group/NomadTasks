:: /*
:: * All or portions of this file Copyright (c) NOMAD Group<nomad-group.net> or its affiliates or
:: * its licensors.
:: *
:: * For complete copyright and license terms please see the LICENSE at the root of this
:: * distribution (the "License"). All use of this software is governed by the License,
:: * or, if provided, by the license below or the license accompanying this file. Do not
:: * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
:: * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: *
:: */
 
@echo off

:: Invoke premake5 with specified args and vs2019 action
.\tools\premake\bin\win32\premake5 %* vs2019 --file=premake5.lua

:: Pause for 5 seconds and auto-close the command window
timeout /t 3 /nobreak
