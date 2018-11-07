#ifndef ETHSNARKS_UTILS_HPP_
#define ETHSNARKS_UTILS_HPP_

#include <fstream>

#include "ethsnarks.hpp"

#include <libsnark/gadgetlib1/pb_variable.hpp>

namespace ethsnarks {


void print_bytes( const char *prefix, const size_t n_bytes,
		  const uint8_t *in_bytes );

void print_bv( const char *prefix, const libff::bit_vector &vec );

void bv_to_bytes(const libff::bit_vector &in_bits, uint8_t *out_bytes);

bool hex_to_bytes( const char *in_hex, uint8_t *out_bytes, size_t out_sz );

int char2int( const char input );

std::vector<unsigned long> bit_list_to_ints(std::vector<bool> bit_list,
					    const size_t wordsize);

libff::bit_vector bytes_to_bv(const uint8_t *in_bytes, const size_t in_count);

VariableArrayT VariableArray_from_bits(
				       ProtoboardT &in_pb,
				       const libff::bit_vector& bits,
				       const std::string &annotation_prefix="" );

void dump_pb_r1cs_constraints(const ProtoboardT& pb);


inline const VariableArrayT make_var_array( ProtoboardT &in_pb, size_t n,
					    const std::string &annotation )
{
    VariableArrayT x;
    x.allocate(in_pb, n, annotation);
    return x;
}


/* `allocate_var_index` is private, must use this workaround... */
inline const VariableT make_variable( ProtoboardT &in_pb,
				      const std::string &annotation )
{
    VariableT x;
    x.allocate(in_pb, annotation);
    return x;
}


/* Multiply a variable by a static number, as a linear combination term */
inline const libsnark::pb_linear_combination<FieldT> make_linear_term(
    ProtoboardT &in_pb,
    VariableT var,
    FieldT coeff )
{
    libsnark::pb_linear_combination<FieldT> lc;
    lc.assign(in_pb, var * coeff);
    return lc;
}


template<typename T>
void writeToFile(std::string path, T& obj) {
    std::stringstream ss;
    ss << obj;
    std::ofstream fh;
    fh.open(path, std::ios::binary);
    ss.rdbuf()->pubseekpos(0, std::ios_base::out);
    fh << ss.rdbuf();
    fh.flush();
    fh.close();
}


template<typename T>
T loadFromFile(std::string path) {
    std::stringstream ss;
    std::ifstream fh(path, std::ios::binary);

    // TODO: more useful error if file not found
    assert(fh.is_open());

    ss << fh.rdbuf();
    fh.close();

    ss.rdbuf()->pubseekpos(0, std::ios_base::in);

    T obj;
    ss >> obj;

    return obj;
}


// namespace ethsnarks
}

#endif
