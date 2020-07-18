#pragma once
#define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
#include <asio.hpp>
#undef _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING

#include "NetConstants.hpp"
#include "NetUtils.hpp"
#include "NetCrypt.hpp"
#include "NetPacketDefinition.hpp"
#include "NetPacket.hpp"
#include "NetPacketDispatcher.hpp"
#include "NetService.hpp"
#include "NetClient.hpp"
#include "NetServer.hpp"
#include "NetPeer.hpp"
#include "NetPeerIdManager.hpp"
#include "NetLogHelper.hpp"
