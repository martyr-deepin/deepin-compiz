import compiz_config_test
import unittest

class CompizConfigProfileTest (compiz_config_test.CompizConfigTest):

    def runTest (self):

        profile = self.ccs.Profile (self.context, "compizconfig")
        self.assertEqual (profile.Name, "compizconfig", 'wrong profile name')
        profile1 = self.ccs.Profile (self.context, "compizconfig2")
        self.assertEqual (profile1.Name, "compizconfig2", 'wrong profile name')
        profile2 = self.ccs.Profile (self.context, "compizconfig3")
        self.assertEqual (profile2.Name, "compizconfig3", 'wrong profile name')

if __name__ == '__main__':
    unittest.main()
