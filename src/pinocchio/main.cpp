#include "circuit_reader.hpp"
#include "stubs.hpp"

using ethsnarks::ppT;
using ethsnarks::CircuitReader;
using ethsnarks::ProtoboardT;
using ethsnarks::stub_prove_from_pb;
using ethsnarks::stub_genkeys_from_pb;
using ethsnarks::stub_main_verify;

using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::string;


static int main_genkeys( ProtoboardT& pb,
			 const char *arith_file,
			 const char *pk_raw,
			 const char *vk_json )
{
	CircuitReader circuit(pb, arith_file, nullptr);

	if( ! pb.is_satisfied() ) {
		cerr << "Error: not satisfied!" << endl;
	}

	return stub_genkeys_from_pb(pb, pk_raw, vk_json);
}


static int main_prove( ProtoboardT& pb,
		       const char *arith_file,
		       const char *circuit_inputs,
		       const char* pk_raw,
		       const char *proof_json )
{
	CircuitReader circuit(pb, arith_file, circuit_inputs);

	if( ! pb.is_satisfied() ) {
		cerr << "Error: not satisfied!" << endl;
	}

    auto json = stub_prove_from_pb(pb, pk_raw);

    ofstream fh;
    fh.open(proof_json, std::ios::binary);
    fh << json;
    fh.flush();
    fh.close();

	return 0;
}


static int main_test( ProtoboardT& pb,
		      const char *arith_file,
		      const char *circuit_inputs )
{
	CircuitReader circuit(pb, arith_file, circuit_inputs);

	if( ! ethsnarks::stub_test_proof_verify(pb) ) {
		cerr << "Error: failed to test!" << endl;
		return  2;
	}

	return 0;
}


static int main_eval( ProtoboardT& pb,
		      const char *arith_file,
		      const char *circuit_inputs,
		      bool traceEnabled )
{
	CircuitReader circuit(pb, arith_file, circuit_inputs, traceEnabled);

	if( ! pb.is_satisfied() ) {
		cerr << "Error: not satisfied!" << endl;
	}

	for( auto& wire : circuit.getOutputWireIds() )
	{
		const auto& value = circuit.varValue(wire);
		cout << wire << "=";
		value.print();
	}

	return 0;
}


int main(int argc, char **argv)
{
	ProtoboardT pb;
	ppT::init_public_params();

	const string progname(argv[0]);
	const string usage_prefix(string("Usage: ")
				  + progname
				  + " <circuit.arith> ");
	if( argc < 3 ) {
		cerr << usage_prefix
		     << "<genkeys|prove|verify|eval|trace|test>"
		     << endl;
		return 1;
	}

	const char *arith_file = argv[1];
	const string cmd(argv[2]);

	int sub_argc = argc - 3;
	const char **sub_argv = (const char**)&argv[3];

	if( cmd == "genkeys" ) {
		if( sub_argc < 2 ) {
			cerr << usage_prefix
			     << cmd
			     << " <proving-key.raw> <verification-key.json>"
			     << endl;
			return 5;
		}
		const char *pk_raw = sub_argv[0];
		const char *vk_json = sub_argv[1];
		return main_genkeys(pb, arith_file, pk_raw, vk_json );
	}
	else if( cmd == "prove" ) {
		if( sub_argc < 3 ) {
			cerr << usage_prefix
			     << cmd
			     << " <circuit.inputs> <proving-key.raw> <output-proof.json>"
			     << endl;
			return 5;
		}
		const char *circuit_inputs = sub_argv[0];
		const char *pk_raw = sub_argv[1];
		const char *proof_json = sub_argv[2];
		return main_prove(pb, arith_file, circuit_inputs, pk_raw, proof_json );
	}
	else if( cmd == "verify" ) {
		if( sub_argc < 2 ) {
			cerr << usage_prefix
			     << cmd
			     << " <verification-key.json> <proof.json>"
			     << endl;
			return 5;
		}
		return stub_main_verify("", 3, (const char**)&argv[2]);
	}
	else if( cmd == "test" ) {
		if( sub_argc == 0 ) {
			cerr << usage_prefix << cmd << " <circuit.inputs>" << endl;
			return 5;
		}
		const char *circuit_inputs = sub_argv[0];
		return main_test(pb, arith_file, circuit_inputs);
	}
	else if( cmd == "eval" || cmd == "trace" ) {
		if( sub_argc == 0 ) {
			cerr << usage_prefix << cmd << " <circuit.inputs>" << endl;
			return 5;
		}
		const char *circuit_inputs = sub_argv[0];
		return main_eval(pb, arith_file, circuit_inputs, cmd == "trace");
	}

	cerr << "Error: unknown sub-command " << cmd << "\n";
	return 2;
}
