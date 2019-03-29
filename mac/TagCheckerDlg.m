#import "TagCheckerDlg.h"
#include "TagCheckerPI.h"

TagCheckerDlg *gTagCheckerDlg = NULL;

@implementation TagCheckerDlg

+ (TagCheckerDlg *) tagCheckerDlg
{
    return gTagCheckerDlg;
}

- (id) init
{
    self = [super init];
    if (![NSBundle loadNibNamed:@"TagCheckerDlg" owner:self])
    {
        AVAlertNote("failed to load TagCheckerDlg.nib");
    }
    gTagCheckerDlg = self;
   
    [Found_1_1 setState: DoAllignSEWithMC()];
    [Found_1_2 setState: DoActualTextNullTerminator()];
    [Found_2_1 setState: DoClassMap()];
    [Found_2_2 setState: DoRoleMap()];
    [Found_2_3 setState: DoIDTree()];
    [Found_2_4 setState: DoAttributes()];
    [Found_2_5 setState: DoTitleEntries()];
    [Found_2_6 setState: DoIDEntries()];
    [Found_2_7 setState: DoOutputIntents()];
    [Found_2_8 setState: DoAcroform()];
    [Found_2_9 setState: DoRedundantLangAttribute()];
    [Found_2_10 setState: DoOutlines()];
    [Found_2_11 setState: DoExtensions()];
    [Found_2_12 setState: DoPageLayout()];
    
    [Fix_1_1 setEnabled: [Found_1_1 state]];
    [Fix_1_2 setEnabled: [Found_1_2 state]];
    [Fix_2_1 setEnabled: [Found_2_1 state]];
    [Fix_2_2 setEnabled: [Found_2_2 state]];
    [Fix_2_3 setEnabled: [Found_2_3 state]];
    [Fix_2_4 setEnabled: [Found_2_4 state]];
    [Fix_2_5 setEnabled: [Found_2_5 state]];
    [Fix_2_6 setEnabled: [Found_2_6 state]];
    [Fix_2_7 setEnabled: [Found_2_7 state]];
    [Fix_2_8 setEnabled: [Found_2_8 state]];
    [Fix_2_9 setEnabled: [Found_2_9 state]];
    [Fix_2_10 setEnabled: [Found_2_10 state]];
    [Fix_2_11 setEnabled: [Found_2_11 state]];
    [Fix_2_12 setEnabled: [Found_2_12 state]];
    return self;
}

- (int) showWindow
{
    [TagCheckerWindow makeKeyAndOrderFront:nil];
    int ret = [[NSApplication sharedApplication] runModalForWindow:TagCheckerWindow];
    [TagCheckerWindow orderOut:self];
    return ret;
}

- (IBAction)okPressed:(id)sender {
    DoAllignSEWithMC( [Fix_1_1 state]);
    DoActualTextNullTerminator( [Fix_1_2 state]);
    
    DoClassMap( [Fix_2_1 state]);
    DoRoleMap( [Fix_2_2 state]);
    DoIDTree( [Fix_2_3 state]);
    DoAttributes( [Fix_2_4 state]);
    DoTitleEntries( [Fix_2_5 state]);
    DoIDEntries( [Fix_2_6 state]);
    DoOutputIntents( [Fix_2_7 state]);
    DoAcroform( [Fix_2_8 state]);
    DoRedundantLangAttribute( [Fix_2_9 state]);
    DoOutlines( [Fix_2_10 state]);
    DoExtensions( [Fix_2_11 state]);
    DoPageLayout( [Fix_2_12 state]);
    
    if ([Wipe_1 state])
        CleanDocumentCatalog();
    if ([Wipe_2 state])
        CleanViewerPreferences();
    
    [NSApp stopModalWithCode:1];
}

- (IBAction)cancelPressed:(id)sender {
    [NSApp stopModalWithCode:0];
}

@end
// end TagCheckerDlg.m
