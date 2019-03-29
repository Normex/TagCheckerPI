# TagCheckerPI
Plug-in for Adobe Acrobat. Checking and fixing pdf files for the purpose of PDF Techniques Accessibility LWG. The plug-in should address the technical problems to be avoided in all sample files created.

The current implementation is able to to perform following tasks:
* Align Strucuture Elements with Container types (checks every structure element that is associated with content object and changes the marked content container tag to match structure element type)
* Remove empty Class Map (only removes ClassMap entry if it is empty. If there are entries in it, we don't touch it)
* Remove empty Role Map (only removes RoleMap entry if it is empty. If there are entries in it, we don't touch it)
* Remove empty Attributes on Structure Elements (removes "A" entry from every structure element only if the "A" dictionary is empty)
* Remove empty Title on Structure Elements (removes "T" entry from every structure element only if the "T" is an empty string)
* Remove empty ID on Structure Elements (removes "ID" entry from every structure element only if the "ID" is an empty string)
* Remove Output Intents entry (OutputIntents dictionary from catalog is removed if exists)
* Remove Acroform entry (if Fields entry in AcroFom is empty array, we remove whole AcroForm entry in catalog dictionary)
* Remove empty ID Tree
* Remove redundant Lang attribute on Structure Elements
* Remove empty Outlines
* Remove Extensions from the document catalog
* Remove empty Page Layout
* Remove the null terminator from Actual Text
* Remove the null terminator from Alternate Text
* Wipe the Document Catalog dictonary removing all entries except Lang, MarkInfo, Metadata, Pages, StructTreeRoot, Type, ViewerPreferences
* Wipe the ViewerPreferences dictionary in the Document Catalog except DisplayDocTitle

Not yet implemented, but planned tasks:
* Fonts not embedded or subsetted
* Uncompressed content streams
* Uncompressed object streams

Not sure about these:
* Unaligned content and logical ordering (unless it serves the purposes of the example)
* Fail cases with the PDF/UA flag
* Pass cases without the PDF/UA flag
* Unnecessary owner dictionaries

# Licensing
Keep in mind that this is not a tool for professional usage. We can't guarantee anything. If you want to use it for different purposes than generating files for PDF Techniques Accessibility LWG, please let us know.

# Documentation

## Screenshot
![alt text](https://github.com/Normex/TagCheckerPI/blob/master/tagchecker.png "TagChecker Windows")
![alt text](https://github.com/Normex/TagCheckerPI/blob/master/tagchecker_mac.png "TagChecker Mac")

The dialog is accessible only on active document from Adobe Acrobat menu Plug-ins -> Tag Checker. Plug in checks all the tasks and indicates with check in the first column if the fix is possible or needed. You then select which tasks you want to run by selecting them in second column. Click OK to perfom fixes and close the dialog. You do this repeatedly. Don't forget to save (or save as) the file.

# Quick Start

## Windows:
 Download plug in from bin folder (https://github.com/Normex/TagCheckerPI/blob/master/bin/TagCheckerPI.api ) to your Adobe Acrobat plug in folder (usually: c:\Program Files (x86)\Adobe\Acrobat DC\Acrobat\plug_ins\ )

## Mac:
Download plug in from bin folder (https://github.com/Normex/TagCheckerPI/blob/master/bin/TagCheckerPI.acroplugin.zip ) and unzip to your Adobe Acrobat plug in folder (usually: /Users/(username)/Library/Application Support/Adobe/Acrobat/DC/Plug-ins)

## Compilation
Whoever wants to compile plug in from sources probably knows what to do :) We used VS 2015 and XCode 10.1, but newer version should compile that just fine.

# Feedback, participate
Feel free to submit comments, questions, suggestions either to github issues section or PDF Techniques Accessibility LWG mailing list or directly to us.
Do you want to add your own tasks? new functionality? Please use pull requests.
