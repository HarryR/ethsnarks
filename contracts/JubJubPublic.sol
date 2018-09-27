pragma solidity ^0.4.24;

import "./JubJub.sol";

contract JubJubPublic
{
	function pointAddViaEtec(uint256[2] a, uint256[2] b)	
		public view returns (uint256[2])
	{
		uint256[4] memory p;
		uint256[4] memory q;
		uint256[4] memory r;
		(p[0], p[1], p[2]) = JubJub.pointToEac(a[0], b[0]);
		(q[0], q[1], q[2]) = JubJub.pointToEac(a[0], b[0]);
		p[3] = 1;
		q[3] = 1;
		r = JubJub.etecAdd(p, q);

		(p[0], p[1]) = JubJub.etecToPoint(r[0], r[1], r[2], r[3]);
		return [p[0], p[1]];
	}

	function pointAdd(uint256[2] a, uint256[2] b)	
		public view returns (uint256[2])
	{
		return JubJub.pointAdd(a, b);
	}


	function pointDoubleViaEtec(uint256[2] p)
		public view returns (uint256 x, uint256 y)
	{
		uint256 t;
		uint256 z;
		(x, y, t) = JubJub.pointToEac(p[0], p[1]);
		(x, y, t, z) = JubJub.etecDouble(x, y, t, 1);
		return JubJub.etecToPoint(x, y, t, z);
	}
}
