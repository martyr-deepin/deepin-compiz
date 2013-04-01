import compiz_config_test
import unittest

class CompizConfigTestPlugin (compiz_config_test.CompizConfigTest):

    def runTest (self):
	plugin = self.ccs.Plugin (self.context, "opengl")
	plugin.Update ()

	self.assertEqual (plugin.Context, self.context)
	self.assertTrue (plugin.Groups is not None)
	self.assertTrue (plugin.Screen is not None)
	self.assertEqual (plugin.Name, "opengl")
	self.assertTrue (plugin.ShortDesc is not None)
	self.assertTrue (plugin.LongDesc is not None)
	self.assertTrue (plugin.Category is not None)
	self.assertTrue (plugin.Features is not None)

if __name__ == '__main__':
    unittest.main()
