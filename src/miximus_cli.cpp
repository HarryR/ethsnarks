// Copyright (c) 2018 HarryR
// License: GPL-3.0+

#include <cstring>
#include <iostream> // cerr
#include <fstream>  // ofstream

#include "mod/miximus.cpp"
#include "stubs.hpp"
#include "utils.hpp" // hex_to_bytes


using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;

using ethsnarks::stub_main_genkeys;
using ethsnarks::stub_main_verify;
using ethsnarks::mod_miximus;


static int main_prove( int argc, char **argv )
{
    if( argc < (9 + (int)MIXIMUS_TREE_DEPTH) )
    {
        cerr << "Usage: "
	     << argv[0]
	     << " prove <pk.raw> <proof.json> <public:root> <public:nullifier> "
             << "<public:exthash> <secret:preimage> <secret:merkle-address> "
	     <<"<secret:merkle-path ...>" << endl;
        cerr << "Args: " << endl;
        cerr << "\t<pk.raw>         Path to proving key" << endl;
        cerr << "\t<proof.json>     Write proof to this file" << endl;
        cerr << "\t<root>           Merkle tree root" << endl;
        cerr << "\t<nullifier>      Nullifier" << endl;
        cerr << "\t<exthash>        Hash of external variables" << endl;
        cerr << "\t<preimage>       Spend preimage" << endl;
        cerr << "\t<merkle-address> 0 and 1 bits for tree path" << endl;
        cerr << "\t<merkle-path...> items for merkle tree path" << endl;
        return 1;
    }

    auto pk_filename = argv[2];
    auto proof_filename = argv[3];
    auto arg_root = argv[4];
    auto arg_nullifier = argv[5];
    auto arg_exthash = argv[6];
    auto arg_preimage = argv[7];
    auto arg_address = argv[8];
    
    const char *arg_path[MIXIMUS_TREE_DEPTH];
    for( size_t i = 0; i < MIXIMUS_TREE_DEPTH; i++ ) {
        arg_path[i] = argv[9 + i];
    }

    auto json = miximus_prove(pk_filename,
			      arg_root,
			      arg_nullifier,
			      arg_exthash,
			      arg_preimage,
			      arg_address,
			      arg_path);

    ofstream fh;
    fh.open(proof_filename, std::ios::binary);
    fh << json;
    fh.flush();
    fh.close();

    return 0;
}


int main( int argc, char **argv )
{
    if( argc < 2 )
    {
        cerr << "Usage: " << argv[0] << " <genkeys|prove|verify> [...]" << endl;
        return 1;
    }

    if( 0 == ::strcmp(argv[1], "prove") )
    {
        return main_prove(argc, argv);
    }
    else if( 0 == ::strcmp(argv[1], "genkeys") )
    {
        return stub_main_genkeys<mod_miximus>(argv[0], argc-1, &argv[1]);
    }
    else if( 0 == ::strcmp(argv[1], "verify") )
    {
        return stub_main_verify(argv[0], argc-1, (const char **)&argv[1]);
    }

    cerr << "Error: unknown sub-command " << argv[1] << endl;
    return 2;
}
