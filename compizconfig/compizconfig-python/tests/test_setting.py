import compiz_config_test
import unittest

class CompizConfigTestSetting (compiz_config_test.CompizConfigTest):

    def runTest (self):

	plugin = self.context.Plugins['core']
	setting = self.ccs.Setting (plugin, 'active_plugins')

	self.assertEqual (setting.Plugin, plugin)
	self.assertEqual (setting.Name, 'active_plugins')
	self.assertEqual (setting.ShortDesc, 'Active Plugins')
	self.assertEqual (setting.LongDesc, 'List of currently active plugins')
	self.assertTrue (setting.Group is not None)
	self.assertTrue (setting.SubGroup is not None)
	self.assertEqual (setting.Type, "List")
	self.assertTrue (setting.Hints is not None)
	self.assertTrue (setting.DefaultValue is not None)
	self.assertTrue (setting.Value is not None)
	self.assertEqual (setting.Integrated, False)
	self.assertEqual (setting.ReadOnly, False)

if __name__ == '__main__':
    unittest.main()
