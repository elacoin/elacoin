// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014 The Spots2 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h"
#include "util.h"
#include "sync.h"
#include "ui_interface.h"
#include "base58.h"
#include "bitcoinrpc.h"
#include "db.h"
//#include "alert.h"
#include "wallet.h"
#include "walletdb.h"

#include <boost/asio.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace json_spirit;

extern CCriticalSection cs_mapAlerts;
extern map<uint256, CAlert> mapAlerts;
 
// send alert.  
// There is a known deadlock situation with ThreadMessageHandler
// ThreadMessageHandler: holds cs_vSend and acquiring cs_main in SendMessages()
// ThreadRPCServer: holds cs_main and acquiring cs_vSend in alert.RelayTo()/PushMessage()/BeginMessage()
Value sendalert(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 6)
        throw runtime_error(
            "sendalert <message> <privatekey> <minver> <maxver> <priority> <id> [expires] [cancelupto]\n"
            "<message> is the alert text message\n"
            "<privatekey> is hex string of alert master private key\n"
            "<minver> is the minimum applicable internal client version\n"
            "<maxver> is the maximum applicable internal client version\n"
            "<priority> is integer priority number\n"
            "<id> is the alert id\n"
            "[expires] is how long to keep the alert active (in seconds)\n"
            "[cancelupto] cancels all alert id's up to this number\n"
            "Returns true or false.");

    CAlert alert;
    CKey key;
    
    int64 nExpires = GetAdjustedTime() + 365*24*60*60;

    alert.strStatusBar = params[0].get_str();
    alert.nMinVer = params[2].get_int();
    alert.nMaxVer = params[3].get_int();
    alert.nPriority = params[4].get_int();
    alert.nID = params[5].get_int();
    if (params.size() > 6)
        nExpires = GetAdjustedTime() + params[6].get_int();
    if (params.size() > 7)
        alert.nCancel = params[7].get_int();
    alert.nVersion = PROTOCOL_VERSION;
    alert.nRelayUntil = nExpires;
    alert.nExpiration = nExpires;

    CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
    sMsg << (CUnsignedAlert)alert;
    alert.vchMsg = vector<unsigned char>(sMsg.begin(), sMsg.end());

    std::vector<unsigned char> tmp = ParseHex(params[1].get_str());
    CPrivKey vchPrivKey(tmp.begin(), tmp.end());
    key.SetPrivKey(vchPrivKey); // if key is not correct openssl may crash
    if (!key.IsValid())
        throw runtime_error("Invalid private key?\n");  
    if (!key.Sign(Hash(alert.vchMsg.begin(), alert.vchMsg.end()), alert.vchSig))
        throw runtime_error(
            "Unable to sign alert, check private key?\n");  
    if(!alert.ProcessAlert()) 
        throw runtime_error(
            "Failed to process alert.\n");
    // Relay alert
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            alert.RelayTo(pnode);
    }

    Object result;
    result.push_back(Pair("strStatusBar", alert.strStatusBar));
    result.push_back(Pair("nVersion", alert.nVersion));
    result.push_back(Pair("nMinVer", alert.nMinVer));
    result.push_back(Pair("nMaxVer", alert.nMaxVer));
    result.push_back(Pair("nPriority", alert.nPriority));
    result.push_back(Pair("nID", alert.nID));
    if (alert.nCancel > 0)
        result.push_back(Pair("nCancel", alert.nCancel));
    return result;
}

Value makekeypair(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "makekeypair [prefix]\n"
            "Make a public/private key pair.\n"
            "[prefix] is optional preferred prefix for the public key.\n");

    string strPrefix = "";
    if (params.size() > 0)
        strPrefix = params[0].get_str();

    CKey key;
    CPubKey pubkey;
    string pubkeyhex;
    CPrivKey privkey;
    string privkeyhex;
    int nCount = 0;
    do
    {
        key.MakeNewKey(false);
        nCount++;
        pubkey = key.GetPubKey();
        std::vector<unsigned char> keys = pubkey.Raw();
        pubkeyhex = HexStr(keys.begin(), keys.end());
        privkey = key.GetPrivKey();
        privkeyhex = HexStr(privkey.begin(), privkey.end());
    } while (nCount < 10000 && strPrefix != pubkeyhex.substr(0, strPrefix.size()));

    if (strPrefix != pubkeyhex.substr(0, strPrefix.size()))
        return Value::null;

    Object result;
    result.push_back(Pair("PublicKey", pubkeyhex));
    result.push_back(Pair("PrivateKey", privkeyhex));
    return result;
}
