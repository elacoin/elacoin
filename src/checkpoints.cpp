// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2012 Elacoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (     0, uint256("0x521d2bc9fdb2534f26e5df423db7a64da14892993e4b4f0329c829a5e8194305"))
        (  2500, uint256("0xd301c065649cbbfb1e93cec353047a12256365c9d50ac689b23ad913df25b6fd"))
        (  5000, uint256("0x7671812122b751155ca6ed038f7b48e0f4f98a27a40dc9521d1c61d98e587544"))
        ( 19200, uint256("0x0d0ce80dfac71b50c597a72bc4ca23f4c8ced8e6c3276716a01ba9c80b353c1d"))
    ( 19350, uint256("0xc6c725fb3c376a2cc4a0796ab4aa04af1ff472628c8e82ece55657b60b3150a5"))
    ( 19400, uint256("0xce70abf9c0930d5cdcff555ceec6e6777f70f3a8bbc4b1efc9ac5c49a3b6c8e6"))
    ( 20940, uint256("0x10478476fb1dc7533d9814e23329633d9758e80a3e2c4b61c6e95778a00e1375"))
    ( 21600, uint256("0xd464161f345ccc1324912ee207db807d2e404f681e2020bcd55ee9f535c0093d"))
    ( 21980, uint256("0xf7934a117422a8efdf91be08194380db701bf9c6387c028175fc4e1d77d25e95"))
    ( 22090, uint256("0xdfb5a7dad007e8ac5cac50ef02943311ae62df054394aba952865cd5a1fe1574"))
        ;

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        MapCheckpoints::const_iterator i = mapCheckpoints.find(nHeight);
        if (i == mapCheckpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0;

        return mapCheckpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
