import unittest
from os import urandom
from random import randint

from ethsnarks.jubjub import Point
from ethsnarks.pedersen import pedersen_hash_points, pedersen_hash_scalars, pedersen_hash_bytes, pedersen_hash_zcash_scalars


class TestPedersenHash(unittest.TestCase):
	def test_bytes(self):
		d = urandom(randint(1, 256))
		p = pedersen_hash_bytes(b'test', d)
		q = pedersen_hash_bytes(b'test', d, d)
		self.assertTrue(p.valid())
		self.assertTrue(q.valid())
		self.assertNotEqual(p, q)

	def test_points(self):
		d = urandom(10)
		p = Point.from_hash(d)
		q = pedersen_hash_points(b'test', p)
		r = pedersen_hash_points(b'test', q)
		self.assertTrue(q.valid())
		self.assertTrue(r.valid())
		self.assertNotEqual(q, r)
	
	def test_zcash(self):
		d = randint(1, 1024)
		#p = pedersen_hash_zcash_scalars(b'test', d)
		#q = pedersen_hash_zcash_scalars(b'test', d, d)
		print(pedersen_hash_zcash_scalars(b'test', 6453482891510615431577168724743356132495662554103773572771861111634748265227))
		self.assertTrue(p.valid)
		self.assertTrue(q.valid)
		self.assertNotEqual(p, q)

if __name__ == "__main__":
	unittest.main()
