import unittest


from ethsnarks.utils import genMerkelTree, getMerkelProof, sha256, initMerkleTree, hashPadded, libsnark2python


def verifyMerkleProof(root, path, address_bits):
    for i, node in enumerate(path):
        bit = address_bits[i]
        if bit:
            the_node = sha256(node, )


class TestMerkleTree(unittest.TestCase):
    def test_tree(self):
        tree_depth = 5
        leaves, nullifiers, sks = initMerkleTree(tree_depth) 
        root, tree = genMerkelTree(tree_depth, leaves)
        for address, (nullifier , sk) in enumerate(zip(nullifiers, sks)):
            path, address_bits = getMerkelProof(leaves, address, tree_depth)
            print(root, path, address_bits)
            #print(verifyMerkleProof(root, path, address_bits))
            # TODO: verify merkle proof

    def test_getMerkelProof(self):
        proof1, address1 =  getMerkelProof([
                "0x0000000000000000000000000000000000000000000000000000000000000000",
                "0x0000000000000000000000000000000000000000000000000000000000000000",
                "0x0000000000000000000000000000000000000000000000000000000000000000",
                "0x0000000000000000000000000000000000000000000000000000000000000000"],
                0, 2)
        self.assertEqual(proof1[0], "0x0000000000000000000000000000000000000000000000000000000000000000")
        self.assertEqual(proof1[1], "0xf5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b")
        self.assertEqual(address1[0], 0)
        self.assertEqual(address1[1], 0)

    def test_GenMerkelTree(self):
        mr1, tree = genMerkelTree(1, ["0x0000000000000000000000000000000000000000000000000000000000000000",
                                      "0x0000000000000000000000000000000000000000000000000000000000000000"])

        mr2, tree = genMerkelTree(2, ["0x0000000000000000000000000000000000000000000000000000000000000000",
                                      "0x0000000000000000000000000000000000000000000000000000000000000000", 
                                      "0x0000000000000000000000000000000000000000000000000000000000000000",
                                      "0x0000000000000000000000000000000000000000000000000000000000000000"])

        mr3, tree = genMerkelTree(29, ["0x0000000000000000000000000000000000000000000000000000000000000000",
                                       "0x0000000000000000000000000000000000000000000000000000000000000000"])

        self.assertEqual(mr1, "0xf5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b") 

        self.assertEqual(mr2, "0xdb56114e00fdd4c1f85c892bf35ac9a89289aaecb1ebd0a96cde606a748b5d71")


    def testHashPadded(self):
        left = "0x0000000000000000000000000000000000000000000000000000000000000000"
        right = "0x0000000000000000000000000000000000000000000000000000000000000000"
        res = hashPadded(left , right)
        self.assertEqual(res, "0xf5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b")


    def testlibsnarkTopython(self):
        inputs = [12981351829201453377820191526040524295325907810881751591725375521336092323040, 
                  2225095499654173609649711272123535458680077283826030252600915820706026312895, 
                  10509931637877506470161905650895697133838017786875388895008260393592381807236, 
                  11784807906137262651861317232543524609532737193375988426511007536308407308209, 17]

        inputs = [9782619478414927069440250629401329418138703122237912437975467993246167708418,
                  2077680306600520305813581592038078188768881965413185699798221798985779874888,
                  4414150718664423886727710960459764220828063162079089958392546463165678021703,
                  7513790795222206681892855620762680219484336729153939269867138100414707910106,
                  902]

        output = libsnark2python(inputs)
        self.assertEqual(output[0], "0x40cde80490e78bc7d1035cbc78d3e6be3e41b2fdfad473782e02e226cc2305a8")
        self.assertEqual(output[1], "0x918e88a16d0624cd5ca4695bd84e23e4a6c8a202ce85560d3c66d4ed39bf4938")
        self.assertEqual(output[2], "0x8dd3ea28fe8d04f3e15b787fec7e805e152fe7d3302d0122c8522bee1290e4b7")
        self.assertEqual(output[3], "0x47a6bbcf8fa3667431e895f08cbd8ec2869a31698d9cf91e5bfd94cbca72161c")


if __name__ == "__main__":
    unittest.main()