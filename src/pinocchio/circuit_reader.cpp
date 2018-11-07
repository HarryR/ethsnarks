/*
MIT License

Copyright (c) 2015 Ahmed Kosba
Copyright (c) 2018 HarryR

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "circuit_reader.hpp"
#include "utils.hpp"
#include "gadgets/lookup_1bit.cpp"
#include "gadgets/lookup_2bit.cpp"
#include "gadgets/lookup_3bit.cpp"
#include "libsnark/gadgetlib1/gadgets/basic_gadgets.hpp"

#include <fstream>


using std::istringstream;
using std::ifstream;
using std::string;
using std::cout;
using std::endl;

using libff::enter_block;
using libff::leave_block;

using libsnark::generate_boolean_r1cs_constraint;

namespace ethsnarks {


static void readIds(char* str, std::vector<unsigned int>& vec)
{
	istringstream iss_i(str, istringstream::in);
	unsigned int id;
	while (iss_i >> id) {
		vec.push_back(id);
	}
}


static void readTable(char* str, std::vector<FieldT>& vec)
{
	istringstream iss_i(str, istringstream::in);
	string token;
	while (iss_i >> token) {
		vec.push_back(FieldT(token.c_str()));
	}
}


static const FieldT readFieldElementFromHex(const char* inputStr){
	char constStrDecimal[150];
	mpz_t integ;
	mpz_init_set_str(integ, inputStr, 16);
	mpz_get_str(constStrDecimal, 10, integ);
	mpz_clear(integ);
	return FieldT(constStrDecimal);
}


CircuitReader::CircuitReader(
	ProtoboardT& in_pb,
	const char* arithFilepath,
	const char* inputsFilepath,
	bool in_traceEnabled
) :
	GadgetT(in_pb, "CircuitReader"),
	traceEnabled(in_traceEnabled)
{
	parseCircuit(arithFilepath);

	if( inputsFilepath ) {
		parseInputs(inputsFilepath);

		if( traceEnabled ) {
			enter_block("Evaluating instructions");
		}

		for( const auto& inst : instructions ) {
			evalInstruction(inst);
		}

		if( traceEnabled ) {
			leave_block("Evaluating instructions");
		}
	}

	makeAllConstraints();
}

/**
* Parse file containing inputs, one line at a time, each line is two numbers:
*
* 	<wire-id> <value>
*/
void CircuitReader::parseInputs( const char *inputsFilepath )
{
	ifstream inputfs(inputsFilepath, ifstream::in);
	string line;

	if (!inputfs.good()) {
		std::cerr << "Unable to open input file: " << inputsFilepath
			  << std::endl;
		exit(-1);
	}
	else {
		char* inputStr;
		while (getline(inputfs, line))
		{
			if (line.length() == 0) {
				continue;
			}
			Wire wireId;
			inputStr = new char[line.size()];
			char separator[2];
			if (3 == sscanf(line.c_str(), "%u%[= ]%s", &wireId,
					separator, inputStr)) {
				const auto value = readFieldElementFromHex(inputStr);
				varSet(wireId, value);
			}
			else {
				std::cerr << "Error in Input" << endl;
				exit(-1);
			}
			delete[] inputStr;
		}
		inputfs.close();
	}
}


void CircuitReader::evalInstruction( const CircuitInstruction &inst )
{
	const auto opcode = inst.opcode;
	const auto& outWires = inst.outputs;
	const auto& constant = inst.constant;

	std::vector<FieldT> inValues;
	for( auto& wire : inst.inputs ) {
		inValues.push_back( varValue(wire));
	}

	if (opcode == ADD_OPCODE) {
		FieldT sum;
		for (auto &v : inValues) {
			sum += v;
		}
		varSet(outWires[0], sum, "add, [input + [input ...]] = C");
	}
	else if (opcode == MUL_OPCODE) {
		varSet(outWires[0], inValues[0] * inValues[1],
		       "mul, A * B = C");
	}
	else if (opcode == XOR_OPCODE) {
		varSet(outWires[0], (inValues[0] == inValues[1])
		       ? FieldT::zero()
		       : FieldT::one(), "xor, A ^ B = C");
	}
	else if (opcode == OR_OPCODE) {
		varSet(outWires[0], (inValues[0] == FieldT::zero()
				     && inValues[1] == FieldT::zero())
		       ? FieldT::zero()
		       : FieldT::one()
		       , "or, A | B = C");
	}
	else if (opcode == ZEROP_OPCODE) {
		varSet(outWires[1], (inValues[0] == FieldT::zero())
		       ? FieldT::zero()
		       : FieldT::one()
		       , "zerop");
	}
	else if (opcode == PACK_OPCODE) {
		FieldT sum;
		FieldT two = FieldT::one();
		for (auto &v : inValues) {
			sum += two * v;
			two += two;
		}
		varSet(outWires[0], sum, "pack");
	}
	else if (opcode == SPLIT_OPCODE) {
		int size = outWires.size();
		FieldT& inVal = inValues[0];
		for (int i = 0; i < size; i++) {
			varSet(outWires[i], inVal.as_bigint().test_bit(i),
			       FMT("split_", "%d", i));
		}
	}
	else if (opcode == CONST_MUL_NEG_OPCODE ) {
		varSet(outWires[0], constant * inValues[0],
		       "const-mul-neg, A * -constant = C");
	}
	else if( opcode == CONST_MUL_OPCODE) {
		varSet(outWires[0], constant * inValues[0],
		       "const-mul, A * constant = C");
	}
	else if( opcode == TABLE_OPCODE ) {
		unsigned int idx = 0;
		for( unsigned int i = 0; i < inValues.size(); i++ ) {
			const auto& val
			  = inValues[inValues.size() - 1 - i].as_ulong();
			assert( val == 0 || val == 1 );
			idx += idx + val;
		}

		varSet(outWires[0], inst.table[idx], "table lookup");
	}
}


void CircuitReader::parseCircuit(const char* arithFilepath)
{
	if( traceEnabled ) {
		enter_block("Parsing Circuit");
	}

	ifstream arithfs(arithFilepath, ifstream::in);
	string line;

	if (!arithfs.good()) {
		std::cerr << "Unable to open circuit file" << arithFilepath
			  << std::endl;
		exit(-1);
	}

	getline(arithfs, line);
	int ret = sscanf(line.c_str(), "total %zu", &numWires);

	if (ret != 1) {
		std::cerr << "File Format Does not Match" << endl;;
		exit(-1);
	}

	char type[200];	// XXX: buffer overflow!
	char* inputStr;
	char* outputStr;
	char* tableStr;
	unsigned int numGateInputs, numGateOutputs;

	// Parse the circuit: few lines were imported from Pinocchio's code.
	while (getline(arithfs, line))
	{
		if (line.length() == 0) {
			continue;
		}
		inputStr = new char[line.size()];
		outputStr = new char[line.size()];
		tableStr = new char[line.size()];

		Wire wireId;
		if (line[0] == '#') {
			continue;
		}
		else if (1 == sscanf(line.c_str(), "input %u", &wireId)) {
			// XXX: public inputs need to go first!
			numInputs++;
			varNew(wireId, FMT("input_", "%zu", wireId));
			inputWireIds.push_back(wireId);
		}
		else if (1 == sscanf(line.c_str(), "nizkinput %u", &wireId)) {
			numNizkInputs++;
			varNew(wireId, FMT("nizkinput_", "%zu", wireId));
			nizkWireIds.push_back(wireId);
		}
		else if (1 == sscanf(line.c_str(), "output %u", &wireId)) {
			numOutputs++;
			varNew(wireId, FMT("output_", "%zu", wireId));
			outputWireIds.push_back(wireId);
		}
		else if (4 == sscanf(line.c_str(),
				     "table %u <%[^>]> in <%[^>]> out <%[^>]>",
				     &numGateInputs,
				     tableStr,
				     inputStr,
				     outputStr)) {
			InputWires inWires;
			OutputWires outWires;
			readIds(inputStr, inWires);
			readIds(outputStr, outWires);
			numGateOutputs = outWires.size();

			if( numGateInputs != inWires.size() ) {
				std::cerr << "Error parsing line: " << line
					  << std::endl;
				std::cerr << " input gate mismatch, expected "
					  << numGateInputs << " got "
					  << inWires.size() << std::endl;
				exit(6);
			}

			if( outWires.size() != 1 ) {
			    std::cerr << "Error parsing line: " << line
				      << std::endl;
			    std::cerr << " output gate mismatch, expected 1, got " << outWires.size()
				      << std::endl;
			    exit(6);
			}

			if( numGateInputs <= 0 || numGateInputs > 3u ) {
				std::cerr << "Error parsing line: " << line << std::endl;
				std::cerr << " unsupported lookup table size: " << numGateInputs
					  << std::endl;
				exit(6);
			}

			std::vector<FieldT> table;
			readTable(tableStr, table);
			if( table.size() != (1u<<numGateInputs) ) {
				std::cerr << "Error parsing line: " << line
					  << std::endl;
				std::cerr << " bad number of table entries, got "
					  << table.size()
					  << " expected "
					  << (1<<inWires.size())
					  << std::endl;
				exit(6);
			}
			instructions.push_back({TABLE_OPCODE,
						0,
						inWires,
						outWires,
						table});
		}
		else if (5 == sscanf(line.c_str(), "%s in %u <%[^>]> out %u <%[^>]>",
				     type,
				     &numGateInputs,
				     inputStr,
				     &numGateOutputs,
				     outputStr)) {

			OutputWires outWires;
			InputWires inWires;
			readIds(inputStr, inWires);
			readIds(outputStr, outWires);

			if( numGateInputs != inWires.size() ) {
				std::cerr << "Error parsing line: " << line
					  << std::endl;
				std::cerr << " input gate mismatch, expected "
					  << numGateInputs
					  << " got "
					  << inWires.size()
					  << std::endl;
				exit(6);
			}

			if( numGateOutputs != outWires.size() ) {
				std::cerr << "Error parsing line: " << line
					  << std::endl;
				std::cerr << " output gate mismatch, expected "
					  << numGateOutputs
					  << " got "
					  << outWires.size()
					  << std::endl;
				exit(6);
			}

			Opcode opcode;
			FieldT constant;
			if (strcmp(type, "add") == 0) {
				opcode = ADD_OPCODE;
			}
			else if (strcmp(type, "mul") == 0) {
				opcode = MUL_OPCODE;
			}
			else if (strcmp(type, "xor") == 0) {
				opcode = XOR_OPCODE;
			}
			else if (strcmp(type, "or") == 0) {
				opcode = OR_OPCODE;
			}
			else if (strcmp(type, "assert") == 0) {
				opcode = ASSERT_OPCODE;
			}
			else if (strcmp(type, "pack") == 0) {
				opcode = PACK_OPCODE;
			}
			else if (strcmp(type, "zerop") == 0) {
				opcode = ZEROP_OPCODE;
			}
			else if (strcmp(type, "split") == 0) {
				opcode = SPLIT_OPCODE;
			}
			else if (strstr(type, "const-mul-neg-")) {
				opcode = CONST_MUL_NEG_OPCODE;
				char* constStr = type + sizeof("const-mul-neg-")
				  - 1;
				constant = readFieldElementFromHex(constStr)
				  * FieldT(-1);
			}
			else if (strstr(type, "const-mul-")) {
				opcode = CONST_MUL_OPCODE;
				char* constStr = type + sizeof("const-mul-") - 1;
				constant = readFieldElementFromHex(constStr);
			}
			else {
				printf("Error: unrecognized line: %s\n",
				       line.c_str());
				exit(-1);
			}

			instructions.push_back({opcode, constant, inWires,
						outWires});
		}
		else {
			printf("Error: unrecognized line: %s\n", line.c_str());
			assert(0);
		}
		delete[] inputStr;
		delete[] outputStr;
		delete[] tableStr;
	}
	arithfs.close();

	this->pb.set_input_sizes(numInputs);

	if( traceEnabled ) {
		leave_block("Parsing Circuit");
	}
}


void CircuitReader::makeAllConstraints( )
{
	for( const auto& inst : instructions )
	{
		makeConstraints( inst );
	}
}


const char* CircuitInstruction::name( ) const
{
	switch( opcode ) {
		case ADD_OPCODE: return "add";
		case MUL_OPCODE: return "mul";
		case XOR_OPCODE: return "xor";
		case OR_OPCODE: return "or";
		case ASSERT_OPCODE: return "assert";
		case ZEROP_OPCODE: return "zerop";
		case SPLIT_OPCODE: return "split";
		case PACK_OPCODE: return "pack";
		case CONST_MUL_OPCODE: return "const-mul";
		case CONST_MUL_NEG_OPCODE: return "const-mul-neg";
		case TABLE_OPCODE: return "table";
		default: return "unknown";
	}
}


static void printWires( const std::vector<Wire> wire_id_list )
{
	bool first = true;
	cout << "<";
	for( const auto& wire_id : wire_id_list ) {
		if( first ) {
			first = false;
		}
		else {
			cout << " ";
		}
		cout << wire_id;
	}
	cout << ">";
}


static void printTable( const std::vector<FieldT> &table ) {
	bool first = true;
	cout << "<";
	for( const auto& item : table ) {
		if( first ) {
			first = false;
		}
		else {
			cout << " ";
		}
		const auto& value = item.as_bigint();
		::gmp_printf("%Nd", value.data, value.N);
	}
	cout << ">";
}


void CircuitInstruction::print() const
{
	// Display table when necessary
	if( opcode == TABLE_OPCODE ) {
		cout << "table " << inputs.size() << " ";
		printTable(table);
		cout << " in ";
		printWires(inputs);
		cout << " out ";
		printWires(outputs);
		cout << endl;
	}
	else {
		// Display input wires
		cout << this->name() << " in " << inputs.size() << " ";
		printWires(inputs);

		// Display output wires
		cout << " out " << outputs.size() << " ";
		printWires(outputs);

		// Display constant value, when necessary
		if( opcode == CONST_MUL_NEG_OPCODE || opcode == CONST_MUL_OPCODE ) {
			cout << " constant=";
			constant.print();	// prints newline
		}
		else {
			cout << endl;
		}
	}
}


void CircuitReader::makeConstraints( const CircuitInstruction& inst )
{
	const auto opcode = inst.opcode;
	const auto& inWires = inst.inputs;
	const auto& outWires = inst.outputs;

	if( traceEnabled ) {
		inst.print();
	}

	if ( opcode == ADD_OPCODE ) {
		assert(inWires.size() > 1);
		handleAddition(inWires, outWires);
	}
	else if ( opcode == MUL_OPCODE ) {
		assert(inWires.size() == 2 && outWires.size() == 1);
		addMulConstraint(inWires, outWires);
	}
	else if ( opcode == XOR_OPCODE ) {
		assert(inWires.size() == 2 && outWires.size() == 1);
		addXorConstraint(inWires, outWires);
	}
	else if ( opcode == OR_OPCODE ) {
		assert(inWires.size() == 2 && outWires.size() == 1);
		addOrConstraint(inWires, outWires);
	}
	else if ( opcode == ASSERT_OPCODE ) {
		assert(inWires.size() == 2 && outWires.size() == 1);
		addAssertionConstraint(inWires, outWires);
	}
	else if ( opcode == CONST_MUL_NEG_OPCODE ) {
		assert(inWires.size() == 1 && outWires.size() == 1);
		handleMulNegConst(inWires, outWires, inst.constant);
	}
	else if ( opcode == CONST_MUL_OPCODE ) {
		assert(inWires.size() == 1 && outWires.size() == 1);
		handleMulConst(inWires, outWires, inst.constant);
	}
	else if ( opcode == ZEROP_OPCODE ) {
		assert(inWires.size() == 1 && outWires.size() == 2);
		addNonzeroCheckConstraint(inWires, outWires);
	}
	else if ( opcode == SPLIT_OPCODE ) {
		assert(inWires.size() == 1);
		addSplitConstraint(inWires, outWires);
	}
	else if ( opcode == PACK_OPCODE ) {
		assert(outWires.size() == 1);
		addPackConstraint(inWires, outWires);
	}
	else if( opcode == TABLE_OPCODE ) {
		addTableConstraint(inWires, outWires, inst.table);
	}

	if( traceEnabled )
	{
		// Show input values
		for( auto& input : inst.inputs ) {
			cout << "\tin " << input << " = ";
			varValue(input).print();
		}

		// Show output values
		for( auto& output : inst.outputs ) {
			cout << "\tout " << output << " = ";
			varValue(output).print();
		}
		cout << endl;
	}
}


FieldT CircuitReader::varValue( Wire wire_id )
{
	auto& var = varGet(wire_id);

	return this->pb.val(var);
}


void CircuitReader::varSet( Wire wire_id, const FieldT& value,
			    const std::string& annotation )
{
	this->pb.val(varGet(wire_id, annotation)) = value;
}


bool CircuitReader::varExists( Wire wire_id )
{
	return variableMap.find(wire_id) != variableMap.end();
}


const VariableT& CircuitReader::varNew( Wire wire_id,
					const std::string &annotation )
{
	VariableT v;
	v.allocate(this->pb, annotation);
	variableMap.emplace(wire_id, v);
	return variableMap[wire_id];
}


const VariableT& CircuitReader::varGet( Wire wire_id,
					const std::string &annotation )
{
	if ( ! varExists(wire_id) ) {
		return varNew(wire_id, annotation);
	}
	return variableMap[wire_id];
}


void CircuitReader::addTableConstraint(const InputWires& inputs,
				       const OutputWires& outputs,
				       const std::vector<FieldT> table)
{
	if( table.size() == 2 ) {
		lookup_1bit_constraints(pb,
					table,
					varGet(inputs[0]),
					varGet(outputs[0]),
					"lookup_1bit");
	}
	else if( table.size() == 4 ) {
		std::vector<VariableT> lut_inputs = {varGet(inputs[0]),
						     varGet(inputs[1])};
		lookup_2bit_constraints(pb, table, {lut_inputs.begin(),
						    lut_inputs.end()},
		  varGet(outputs[0]), "lookup_2bit");
	}
	else if( table.size() == 8 ) {
		std::vector<VariableT> lut_inputs = {varGet(inputs[0]),
						     varGet(inputs[1]),
						     varGet(inputs[2])};
		lookup_3bit_gadget lut(pb, table, {lut_inputs.begin(),
						   lut_inputs.end()},
		  "lookup_3bit");
		lut.generate_r1cs_constraints();
	}
}


void CircuitReader::addMulConstraint(const InputWires& inputs,
				     const OutputWires& outputs)
{
	auto& l1 = varGet(inputs[0], FMT("mul A ", "(%zu)", inputs[0]));
	auto& l2 = varGet(inputs[1], FMT("mul B ", "(%zu)", inputs[1]));
	auto& outvar = varGet(outputs[0], FMT("mul out", "%zu", outputs[0]));

	pb.add_r1cs_constraint(ConstraintT(l1, l2, outvar), "mul, A * B = C");
}


void CircuitReader::addXorConstraint(const InputWires& inputs,
				     const OutputWires& outputs)
{
	auto& l1 = varGet(inputs[0], "xor A");
	auto& l2 = varGet(inputs[1], "xor B");
	auto& outvar = varGet(outputs[0], "xor result");

	pb.add_r1cs_constraint(ConstraintT(2 * l1, l2, l1 + l2 - outvar),
			       "xor, A ^ B = C");
}


void CircuitReader::addOrConstraint(const InputWires& inputs,
				    const OutputWires& outputs)
{
	auto& l1 = varGet(inputs[0], "or A");
	auto& l2 = varGet(inputs[1], "or B");
	auto& outvar = varGet(outputs[0], "or result");

	pb.add_r1cs_constraint(ConstraintT(l1, l2, l1 + l2 - outvar),
			       "or, A | B = C");
}


void CircuitReader::addAssertionConstraint(const InputWires& inputs,
					   const OutputWires& outputs)
{
	auto& l1 = varGet(inputs[0], "assert A");
	auto& l2 = varGet(inputs[1], "assert B");
	auto& l3 = varGet(outputs[0], "assert C");

	pb.add_r1cs_constraint(ConstraintT(l1, l2, l3), "assert, A * B = C");
}


void CircuitReader::addSplitConstraint(const InputWires& inputs,
				       const OutputWires& outputs)
{
	LinearCombinationT sum;

	auto two_i = FieldT::one();

	for( size_t i = 0; i < outputs.size(); i++)
	{
		auto &out_bit_var = varGet(outputs[i], FMT("split.output",
							   "[%d][%zu]",
							   outputs[i], i));

		generate_boolean_r1cs_constraint<FieldT>(pb, out_bit_var);

		sum.add_term( out_bit_var * two_i );

		two_i += two_i;
	}

	pb.add_r1cs_constraint(
		ConstraintT(
			varGet(inputs[0], FMT("split.input", "[%d]", inputs[0])), 1, sum),
			"split result");
}


void CircuitReader::addPackConstraint(const InputWires& inputs, const OutputWires& outputs)
{
	LinearCombinationT sum;

	auto two_i = FieldT::one();

	for( size_t i = 0; i < inputs.size(); i++ )
	{
		sum.add_term(varGet(inputs[i], FMT("pack.input", "[%d]", inputs[i])) * two_i);
		two_i += two_i;
	}

	pb.add_r1cs_constraint(
		ConstraintT(
			varGet(outputs[0], FMT("pack.output", "[%d]", outputs[0])), 1, sum),
			"pack");
}


/**
* Zero Equality Gate
* 
* Another useful type of comparison functionality is checking
* whether a value is equal to zero. e.g.
*
*	Y = (X != 0) ? 1 : 0
*
* This is equivalent to satisfying the following two constraints:
*
*	(X * M) - Y = 0
*
* and:
*
*	X * (1 - Y) = 0
*
* in addition to the bitness constraint for Y:
*
*   Y * Y = Y
*
* For any value M, M should be (1.0/X), where `X*M==1` if X is non-zero.
*/
void CircuitReader::addNonzeroCheckConstraint(const InputWires& inputs,
					      const OutputWires& outputs)
{
	auto& X = varGet(inputs[0], FMT("zerop input", " (%zu)", inputs[0]));

	auto& Y = varGet(outputs[0], FMT("zerop output", " (%zu)", outputs[0]));

	VariableT M;
	M.allocate(this->pb, FMT("zerop aux", " (%zu,%zu)", inputs[0], outputs[0]));

	generate_boolean_r1cs_constraint<FieldT>(pb, Y);

	pb.add_r1cs_constraint(ConstraintT(X, 1 - LinearCombinationT(Y), 0),
			       "X is 0, or Y is 1");

	pb.add_r1cs_constraint(ConstraintT(X, M, Y), "X * (1/X) = Y");

	zerop_items.push_back({inputs[0], M});
}


void CircuitReader::handleAddition(const InputWires& inputs,
				   const OutputWires& outputs)
{
	auto& outwire = varGet(outputs[0], "add output");

	LinearCombinationT sum;

	for( auto& input_id : inputs )
	{
		sum.add_term(varGet(input_id));
	}

	pb.add_r1cs_constraint(ConstraintT(1, sum, outwire),
			       "add, [input + [input ...]] = C");
}


void CircuitReader::handleMulConst(const InputWires& inputs,
				   const OutputWires& outputs,
				   const FieldT& constant)
{
	auto& A = varGet(inputs[0], "mul const input");

	auto& C = varGet(outputs[0], "mul const output");

	pb.add_r1cs_constraint(ConstraintT(A, constant, C),
			       "mulconst, A * constant = C");
}


void CircuitReader::handleMulNegConst(const InputWires& inputs,
				      const OutputWires& outputs,
				      const FieldT &constant)
{
	auto& A = varGet(inputs[0], "const-mul-neg input");

	auto& C = varGet(outputs[0], "const-mul-neg output");

	pb.add_r1cs_constraint(ConstraintT(A, constant, C),
			       "mulnegconst, A * -constant = C");
}

// namespace ethsnarks
}
