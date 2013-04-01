import compiz_config_test
import unittest

class CompizConfigTestBackend (compiz_config_test.CompizConfigTest):

    def runTest (self):

	info = ["ini", "Ini Backend", "Flat File Backend", True, False]

	backend = self.ccs.Backend (self.context, info)

	self.assertEqual (backend.Name, "ini")
	self.assertEqual (backend.ShortDesc, "Ini Backend")
	self.assertEqual (backend.LongDesc, "Flat File Backend")
	self.assertEqual (backend.IntegrationSupport, False)
	self.assertEqual (backend.ProfileSupport, True)

if __name__ == '__main__':
    unittest.main()
