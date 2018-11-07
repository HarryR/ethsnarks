#include <libsnark/gadgetlib1/protoboard.hpp>

#include <sstream>  // stringstream

#include "utils.hpp"
#include "import.hpp"
#include "export.hpp"

#include "r1cs_gg_ppzksnark_zok/r1cs_gg_ppzksnark_zok.hpp"

namespace ethsnarks {

bool stub_verify( const char *vk_json, const char *proof_json )
{
    ppT::init_public_params();

    std::stringstream vk_stream;
    vk_stream << vk_json;
    auto vk = vk_from_json(vk_stream);

    std::stringstream proof_stream;
    proof_stream << proof_json;
    auto proof_pair = proof_from_json(proof_stream);

    auto status = libsnark::r1cs_gg_ppzksnark_zok_verifier_strong_IC <ppT> (
	  vk, proof_pair.first, proof_pair.second);
    if( status )
        return true;

    return false;
}


std::string stub_prove_from_pb( ProtoboardT& pb, const char *pk_file )
{
    auto proving_key = ethsnarks::loadFromFile<ethsnarks::ProvingKeyT>(pk_file);
    // TODO: verify if proving key was loaded correctly, if not return NULL

    auto primary_input = pb.primary_input();
    auto proof = libsnark::r1cs_gg_ppzksnark_zok_prover<ethsnarks::ppT>(
	proving_key, primary_input, pb.auxiliary_input());
    return ethsnarks::proof_to_json(proof, primary_input);
}


int stub_genkeys_from_pb( ProtoboardT& pb, const char *pk_file,
			  const char *vk_file )
{
    auto keypair = libsnark::r1cs_gg_ppzksnark_zok_generator<ppT>(
	pb.get_constraint_system());
    vk2json_file(keypair.vk, vk_file);
    writeToFile<decltype(keypair.pk)>(pk_file, keypair.pk);

    return 0;
}


int stub_main_verify( const char *prog_name, int argc, const char **argv )
{
    if( argc < 3 )
    {
        std::cerr << "Usage: " << prog_name << " " << argv[0]
		  << " <vk.json> <proof.json>" << std::endl;
        return 1;
    }

    auto vk_json_file = argv[1];
    auto proof_json_file = argv[2];

    // Read verifying key file
    std::stringstream vk_stream;
    std::ifstream vk_input(vk_json_file);
    if( ! vk_input ) {
        std::cerr << "Error: cannot open " << vk_json_file << std::endl;
        return 2;
    }
    vk_stream << vk_input.rdbuf();
    vk_input.close();

    // Read proof file
    std::stringstream proof_stream;
    std::ifstream proof_input(proof_json_file);
    if( ! proof_input ) {
        std::cerr << "Error: cannot open " << proof_json_file << std::endl;
        return 2;
    }
    proof_stream << proof_input.rdbuf();
    proof_input.close();

    // Then verify if proof is correct
    auto vk_str = vk_stream.str();
    auto proof_str = proof_stream.str();
    if( stub_verify( vk_str.c_str(), proof_str.c_str() ) )
    {
        return 0;
    }

    std::cerr << "Error: failed to verify proof!" << std::endl;

    return 1;
}


bool stub_test_proof_verify( const ProtoboardT &in_pb )
{
    auto constraints = in_pb.get_constraint_system();
    auto keypair = libsnark::r1cs_gg_ppzksnark_zok_generator<ppT>(constraints);

    auto primary_input = in_pb.primary_input();
    auto auxiliary_input = in_pb.auxiliary_input();
    auto proof = libsnark::r1cs_gg_ppzksnark_zok_prover<ppT>(keypair.pk,
							     primary_input,
							     auxiliary_input);

    return libsnark::r1cs_gg_ppzksnark_zok_verifier_strong_IC <ppT> (keypair.vk,
							   primary_input,
							   proof);
}


}
// namespace ethsnarks
