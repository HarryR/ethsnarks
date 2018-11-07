#include <fstream>
#include <iomanip>

#include "utils.hpp"

namespace ethsnarks {


// Copied from `int_list_to_bits`
libff::bit_vector bytes_to_bv(const uint8_t *in_bytes, const size_t in_count)
{
    libff::bit_vector res(in_count * 8);
    for( size_t i = 0; i < in_count; i++ )
    {
        for( size_t j = 0; j < 8; j++ ) {
            res[i * 8 + j] = (in_bytes[i] & (1 << (8 - 1 - j))) ? 1 : 0;
        }
    }
    return res;
}


std::vector<unsigned long> bit_list_to_ints(std::vector<bool> bit_list,
					    const size_t wordsize)
{
    std::vector<unsigned long> res;
    size_t iterations = bit_list.size()/wordsize;

    for (size_t i = 0; i < iterations; ++i)
    {
        unsigned long current = 0;
        for (size_t j = 0; j < wordsize; ++j)
        {
            if (bit_list.size() == (i*wordsize+j)) {
                break;
            }

            current += (bit_list[i*wordsize+j] * (1ul<<(wordsize-1-j)));
      }

      res.push_back(current);
    }

    return res;
}


/**
* Convert a bit vector to a pb_variable_array
*/
VariableArrayT VariableArray_from_bits(
    ProtoboardT &in_pb,
    const libff::bit_vector& bits,
    const std::string &annotation_prefix )
{
    VariableArrayT out;
    out.allocate(in_pb, bits.size(), annotation_prefix);
    out.fill_with_bits(in_pb, bits);
    return out;
}



int char2int( const char input )
{
    if( input >= '0' && input <= '9' )
        return input - '0';

    if( input >= 'A' && input <= 'F')
        return input - 'A' + 10;

    if( input >= 'a' && input <= 'f')
        return input - 'a' + 10;

    throw std::invalid_argument("Invalid hex");
}


/**
* Decode a hexadecimal string `in_hex` representing `out_sz` bytes into `out_bytes`
* The hex string can, optionally, be prefixed with '0x'
*/
bool hex_to_bytes( const char *in_hex, uint8_t *out_bytes, size_t out_sz )
{
    if( ::strlen(in_hex) < 2 )
        return false;

    if( 0 == ::strncmp(in_hex, "0x", 2) )
        in_hex = &in_hex[2];

    size_t hex_sz = strlen(in_hex);

    if( hex_sz % 2 != 0 || (hex_sz / 2) != out_sz )
        return false;

    while( *in_hex ) {
        const char hex0 = in_hex[0];
        const char hex1 = in_hex[1];
        *out_bytes = (uint8_t)( (char2int(hex0) << 4) | char2int(hex1) );
        out_bytes += 1;
        in_hex += 2;
    }

    return true;
}


void bv_to_bytes(const libff::bit_vector &in_bits, uint8_t *out_bytes)
{
    for( auto& b : bit_list_to_ints(in_bits, 8) ) {
        *out_bytes++ = (uint8_t)b;
    }
}


void print_bv( const char *prefix, const libff::bit_vector &vec )
{
    std::cout << prefix << ": ";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        std::cout << vec[i];
        if( i > 0 && i % 8 == 0 ) {
            std::cout << " ";
        }
    }
    std::cout << "\n";
}


void print_bytes( const char *prefix, const size_t n_bytes, const uint8_t *in_bytes )
{
    printf("%s: ", prefix);
    for (size_t i = 0; i < n_bytes; i++)
    {
       printf("%02X", in_bytes[i]);
    }
    printf("\n");
}


void dump_pb_r1cs_constraints(const ProtoboardT& pb)
{
    auto full_variable_assignment = pb.primary_input();
    const auto auxiliary_input = pb.auxiliary_input();
    full_variable_assignment.insert(full_variable_assignment.end(),
				    auxiliary_input.begin(),
				    auxiliary_input.end());

    const auto cs = pb.get_constraint_system();

    unsigned int i = 0;
    for( auto& constraint : cs.constraints )
    {
        const FieldT ares = constraint.a.evaluate(full_variable_assignment);
        const FieldT bres = constraint.b.evaluate(full_variable_assignment);
        const FieldT cres = constraint.c.evaluate(full_variable_assignment);

        auto it = cs.constraint_annotations.find(i);
        printf("constraint %u (%s)\n", i++,
	       (it == cs.constraint_annotations.end()
		? "no annotation"
		: it->second.c_str()));
        printf("\t<a,(1,x)> = "); ares.print();
        printf("\t<b,(1,x)> = "); bres.print();
        printf("\t<c,(1,x)> = "); cres.print();
        printf("constraint was:\n");
        dump_r1cs_constraint(constraint, full_variable_assignment,
			     cs.variable_annotations);
        printf("\n");
    }
}


// ethsnarks
}
