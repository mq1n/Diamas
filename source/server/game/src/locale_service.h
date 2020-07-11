#pragma once

extern void Locale_Init();

bool LocaleService_Init(const std::string& c_rstServiceName);
void LocaleService_TransferDefaultSetting();
const std::string& LocaleService_GetBasePath();
const std::string& LocaleService_GetMapPath();
const std::string& LocaleService_GetQuestPath();
