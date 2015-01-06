#!/usr/bin/env python

import argparse, glob, os, subprocess, sys

def main(args):
	sourceRootLocation = os.path.abspath(args.sourceRoot)
	sourceRootLocation = os.path.join(sourceRootLocation, "")
	testGlobSyntax     = "*_tests"
	globExpression     = sourceRootLocation + testGlobSyntax
	tests = glob.glob(globExpression)
	
	testsSuccessful = True

	for test in tests:
		try:
			subprocess.check_call([test], cwd=args.dataRoot)
		except subprocess.CalledProcessError as e:
			print str(e.cmd) + " failed with return code: " + str(e.returncode)
			testsSuccessful = False

	return testsSuccessful

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Runs enlighten tests')
	parser.add_argument('--sourceRoot', help='path to the root location containing the test executables', required=True)
	parser.add_argument('--dataRoot', help='path to the root of the data folder', required=True)

	args = parser.parse_args()
	success = main(args)

	if not success:
		sys.exit(1)
	else:
		sys.exit(0)
