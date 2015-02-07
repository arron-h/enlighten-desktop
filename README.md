enlighten-desktop [![Build Status](https://travis-ci.org/arron-h/enlighten-desktop.svg?branch=master)](https://travis-ci.org/arron-h/enlighten-desktop)
=================

The desktop component for the [enlighten](https://github.com/arron-h/enlighten) project.

TODO
----
* Fix intermitent bug with threaded access to PreviewsDatabase. This is highlighted in the PreviewsSynchronizer unit test.
* Wrap libS3 so it can easily be mocked and clients tested.
* Integrate RapidJson and load Aws/application settings from it.
* Flesh out AwsRequest.
* Write LrCatSynchronizer.
* Write/think about how source Jpegs are going to be synched!
