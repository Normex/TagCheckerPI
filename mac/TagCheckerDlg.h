#ifndef __TagCheckerDlg_H__
#define __TagCheckerDlg_H__

#import <Cocoa/Cocoa.h>

@interface TagCheckerDlg : NSObject
{
    IBOutlet NSWindow *TagCheckerWindow;
    IBOutlet NSButton *Found_1_1;
    IBOutlet NSButton *Found_1_2;
    IBOutlet NSButton *Found_2_1;
    IBOutlet NSButton *Found_2_2;
    IBOutlet NSButton *Found_2_3;
    IBOutlet NSButton *Found_2_4;
    IBOutlet NSButton *Found_2_5;
    IBOutlet NSButton *Found_2_6;
    IBOutlet NSButton *Found_2_7;
    IBOutlet NSButton *Found_2_8;
    IBOutlet NSButton *Found_2_9;
    IBOutlet NSButton *Found_2_10;
    IBOutlet NSButton *Found_2_11;
    IBOutlet NSButton *Found_2_12;
    IBOutlet NSButton *Fix_1_1;
    IBOutlet NSButton *Fix_1_2;
    IBOutlet NSButton *Fix_2_1;
    IBOutlet NSButton *Fix_2_2;
    IBOutlet NSButton *Fix_2_3;
    IBOutlet NSButton *Fix_2_4;
    IBOutlet NSButton *Fix_2_5;
    IBOutlet NSButton *Fix_2_6;
    IBOutlet NSButton *Fix_2_7;
    IBOutlet NSButton *Fix_2_8;
    IBOutlet NSButton *Fix_2_9;
    IBOutlet NSButton *Fix_2_10;
    IBOutlet NSButton *Fix_2_11;
    IBOutlet NSButton *Fix_2_12;
    IBOutlet NSButton *Wipe_1;
    IBOutlet NSButton *Wipe_2;
}

- (int) showWindow;
+ (TagCheckerDlg *) tagCheckerDlg;
@end

#endif //__TagCheckerDlg_H__
