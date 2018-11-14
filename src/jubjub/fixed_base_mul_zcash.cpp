#include "jubjub/fixed_base_mul_zcash.hpp"

namespace ethsnarks {

namespace jubjub {

const static char chunk_size_bits = 3;
const static char lookup_size_bits = 2;
const static char chunks_per_base_point = 62;

Point add(Point lhs, Point rhs, const Params& in_params) {
	Point result;
	const FieldT x1y2 = lhs.x * rhs.y;
	const FieldT y1x2 = lhs.y * rhs.x;
	const FieldT y1y2 = lhs.y * rhs.y;
	const FieldT x1x2 = lhs.x * rhs.x;
	const FieldT dx1x2y1y2 = in_params.d * x1x2 * y1y2;

	return {
		(x1y2 + y1x2) * (FieldT::one() + dx1x2y1y2).inverse(),
		(y1y2 - (in_params.a * x1x2)) * (FieldT::one() - dx1x2y1y2).inverse()
	};
}

fixed_base_mul_zcash::fixed_base_mul_zcash(
	ProtoboardT &in_pb,
	const Params& in_params,
	const std::vector<Point> base_points,
	const VariableArrayT in_scalar,
	const std::string &annotation_prefix
) :
	GadgetT(in_pb, annotation_prefix)
{
	assert( (in_scalar.size() % chunk_size_bits) == 0 );
	assert( float(in_scalar.size()) / float(chunk_size_bits * chunks_per_base_point) <= base_points.size());
	int window_size_items = 1 << lookup_size_bits;
	int n_windows = in_scalar.size() / chunk_size_bits;

	Point start = base_points[0];
	// Precompute values for all lookup window tables
	for( int i = 0; i < n_windows; i++ )
	{
		std::vector<FieldT> lookup_x;
		std::vector<FieldT> lookup_y;

		if (i % chunks_per_base_point == 0) {
			start = base_points[i/chunks_per_base_point];
		}

		// For each window, generate 4 points, in little endian:
		// (0,0) = 0 = start = base*2^4i
		// (1,0) = 1 = 2*start
		// (0,1) = 2 = 3*start
		// (1,1) = 3 = 4*start
		Point current = start;
		for( int j = 0; j < window_size_items; j++ )
		{
			if (j != 0) {
				current = add(current, start, in_params);
			}
			lookup_x.emplace_back(current.x);
			lookup_y.emplace_back(current.y);
		}

		const auto bits_begin = in_scalar.begin() + (i * chunk_size_bits);
		const VariableArrayT window_bits_x( bits_begin, bits_begin + chunk_size_bits );
		const VariableArrayT window_bits_y( bits_begin, bits_begin + lookup_size_bits );
		m_windows_x.emplace_back(in_pb, lookup_x, window_bits_x, FMT(annotation_prefix, ".windows_x[%d]", i));
		m_windows_y.emplace_back(in_pb, lookup_y, window_bits_y, FMT(annotation_prefix, ".windows_y[%d]", i));

		// current is at 2^2 * start, for next iteration start needs to be 2^4
		current = add(current, current, in_params);
		start = add(current, current, in_params);
	}

	// Chain adders together, adding output of previous adder with current window
	// First adder ads the first two windows together as there is no previous adder
	for( int i = 1; i < n_windows; i++ )
	{
		if( i == 1 ) {				
			m_adders.emplace_back(
				in_pb, in_params,
				m_windows_x[i-1].result(),
				m_windows_y[i-1].result(),
				m_windows_x[i].result(),
				m_windows_y[i].result(),
				FMT(this->annotation_prefix, ".adders[%d]", i));
		}
		else {
			m_adders.emplace_back(
				in_pb, in_params,
				m_adders[i-2].result_x(),
				m_adders[i-2].result_y(),
				m_windows_x[i].result(),
				m_windows_y[i].result(),
				FMT(this->annotation_prefix, ".adders[%d]", i));
		}
	}
}

void fixed_base_mul_zcash::generate_r1cs_constraints ()
{
	for( auto& lut_x : m_windows_x ) {
		lut_x.generate_r1cs_constraints();
	}

	for( auto& lut_y : m_windows_y ) {
		lut_y.generate_r1cs_constraints();
	}

	for( auto& adder : m_adders ) {
		adder.generate_r1cs_constraints();
	}
}

void fixed_base_mul_zcash::generate_r1cs_witness ()
{
	for( auto& lut_x : m_windows_x ) {
		lut_x.generate_r1cs_witness();
	}

	for( auto& lut_y : m_windows_y ) {
		lut_y.generate_r1cs_witness();
	}

	for( auto& adder : m_adders ) {
		adder.generate_r1cs_witness();
	}
}

const VariableT& fixed_base_mul_zcash::result_x() {
	return m_adders[ m_adders.size() - 1 ].result_x();
}

const VariableT& fixed_base_mul_zcash::result_y() {
	return m_adders[ m_adders.size() - 1 ].result_y();
}

// namespace jubjub
}

// namespace ethsnarks
}
