#include "circuit_reader.hpp"
#include "stubs.hpp"

using ethsnarks::ppT;
using ethsnarks::CircuitReader;
using ethsnarks::ProtoboardT;

using std::string;
using std::cerr;
using std::cout;
using std::endl;


int main(int argc, char **argv)
{
	ProtoboardT pb;
	ppT::init_public_params();

	const string usage(string("Usage: ")
			   + argv[0]
			   + " <circuit.arith> <circuit.input>");

	if( argc < 3 ) {
		cerr << usage << endl;
		return 1;
	}

	const char *arith_file = argv[1];
	const char *circuit_inputs = argv[2];

	CircuitReader circuit(pb, arith_file, circuit_inputs, false);

	if( ! pb.is_satisfied() ) {
		cerr << "Error: not satisfied!" << endl;
		return 2;
	}

	for( auto& wire : circuit.getOutputWireIds() )
	{
		const auto& value = circuit.varValue(wire);
		cout << wire << "=";
		value.print();
	}

	return 0;
}
