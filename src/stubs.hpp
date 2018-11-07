#ifndef ETHSNARKS_STUBS_HPP
#define ETHSNARKS_STUBS_HPP

#include "utils.hpp"
#include "export.hpp"

namespace ethsnarks {

bool stub_verify( const char *vk_json, const char *proof_json );

int stub_main_verify( const char *prog_name, int argc, const char **argv );

bool stub_test_proof_verify( const ProtoboardT &in_pb );

int stub_genkeys_from_pb( ProtoboardT& pb, const char *pk_file,
			  const char *vk_file );

std::string stub_prove_from_pb( ProtoboardT& pb, const char *pk_file );


template<class GadgetT>
int stub_genkeys( const char *pk_file, const char *vk_file )
{
    ppT::init_public_params();

    ProtoboardT pb;
    GadgetT mod(pb, "module");
    mod.generate_r1cs_constraints();

    return stub_genkeys_from_pb(pb, pk_file, vk_file);
}


template<class GadgetT>
int stub_main_genkeys( const char *prog_name, int argc, char **argv )
{
    if( argc < 3 )
    {
        std::cerr << "Usage: " << prog_name << " " << argv[0]
		  << " <pk-output.raw> <vk-output.json>" << std::endl;
        return 1;
    }

    auto pk_file = argv[1];
    auto vk_file = argv[2];

    if( 0 != stub_genkeys<GadgetT>( pk_file, vk_file ) )
    {
        std::cerr << "Error: failed to generate proving and verifying keys"
		  << std::endl;
        return 1;
    }

    return 0;
}

}

#endif
