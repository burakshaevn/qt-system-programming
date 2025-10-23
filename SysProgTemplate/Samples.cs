namespace SysProgTemplate 
{
    public static class Samples
    {
        public static string StraightSample { get; set; } =
           @"PROG  START   0
    JMP     L1 
A1  RESB    10
A2  RESW    20 
B1  WORD    4096 
B2  BYTE    X""2F4C008A""
B3  BYTE    C""Hello!""
B4  BYTE    128	
L1  LOADR1  B1	
    LOADR2  B4
    ADD     R1  R2
    SAVER1  L1
    INT     200	
    END 
            ";

        public static string RelativeSample { get; set; } =
   @"PROG  START   0
    JMP     [L1] 
A1  RESB    10
A2  RESW    20 
B1  WORD    4096 
B2  BYTE    X""2F4C008A""
B3  BYTE    C""Hello!""
B4  BYTE    128	
L1  LOADR1  [B1]	
    LOADR2  [B4]
    ADD     R1  R2
    SAVER1  [L1]
    INT     200	
    END 
            ";

        public static string MixedSample { get; set; } =
   @"PROG  START   0
    JMP     [L1] 
A1  RESB    10
A2  RESW    20 
B1  WORD    4096 
B2  BYTE    X""2F4C008A""
B3  BYTE    C""Hello!""
B4  BYTE    128	
L1  LOADR1  B1	
    LOADR2  [B4]
    ADD     R1  R2
    SAVER1  L1
    INT     200	
    END 
            ";

    }
}
