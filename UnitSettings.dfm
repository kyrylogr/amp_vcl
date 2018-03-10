object FormSettings: TFormSettings
  Left = 0
  Top = 0
  Caption = 'Adjust Settings'
  ClientHeight = 591
  ClientWidth = 479
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object LDisabled: TLabel
    Left = 16
    Top = 557
    Width = 256
    Height = 26
    Caption = 
      '(*) Settings can be only changed from the main window when there' +
      ' is no running process'
    WordWrap = True
  end
  object BNOK: TButton
    Left = 252
    Top = 560
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ModalResult = 1
    ParentFont = False
    TabOrder = 4
  end
  object Cancel: TButton
    Left = 350
    Top = 560
    Width = 75
    Height = 25
    Caption = 'Cancel'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ModalResult = 2
    ParentFont = False
    TabOrder = 5
  end
  object GBTimeout: TGroupBox
    Left = 8
    Top = 4
    Width = 448
    Height = 189
    Caption = 'Timeouts'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
    object Label1: TLabel
      Left = 20
      Top = 131
      Width = 74
      Height = 13
      Caption = 'Default timeout'
    end
    object Label2: TLabel
      Left = 20
      Top = 153
      Width = 177
      Height = 26
      Caption = 'Default timeout for complex element (nested contol module XML)'
      WordWrap = True
    end
    object Label3: TLabel
      Left = 270
      Top = 131
      Width = 47
      Height = 13
      Caption = '(seconds)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 270
      Top = 159
      Width = 47
      Height = 13
      Caption = '(seconds)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object EDefaultTimeout: TEdit
      Left = 204
      Top = 127
      Width = 60
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      NumbersOnly = True
      ParentFont = False
      TabOrder = 2
    end
    object EDefaultTimeoutComplex: TEdit
      Left = 204
      Top = 156
      Width = 60
      Height = 21
      NumbersOnly = True
      TabOrder = 3
    end
    object RGEnabled: TRadioGroup
      Left = 12
      Top = 12
      Width = 425
      Height = 89
      Items.Strings = (
        'All enabled'
        'High level disabled (model and controlmodule)'
        'All disabled')
      TabOrder = 0
    end
    object CBTimeoutWithoutWait: TCheckBox
      Left = 20
      Top = 107
      Width = 197
      Height = 17
      Caption = 'Timeout without wait time'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
  end
  object GBLogs: TGroupBox
    Left = 8
    Top = 194
    Width = 448
    Height = 138
    Caption = 'Logs'
    TabOrder = 1
    object Label5: TLabel
      Left = 16
      Top = 41
      Width = 63
      Height = 13
      Caption = 'Log directory'
    end
    object Label6: TLabel
      Left = 16
      Top = 93
      Width = 164
      Height = 13
      Caption = 'Auto delete old log files older than'
    end
    object Label7: TLabel
      Left = 270
      Top = 93
      Width = 31
      Height = 13
      Caption = '(days)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object ELogDirectory: TEdit
      Left = 39
      Top = 60
      Width = 398
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
    object ELogClearPeriod: TEdit
      Left = 204
      Top = 87
      Width = 60
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      NumbersOnly = True
      ParentFont = False
      TabOrder = 2
    end
    object CBWriteCSVLogs: TCheckBox
      Left = 20
      Top = 18
      Width = 113
      Height = 17
      Caption = 'Write CSV logs'
      TabOrder = 0
    end
    object CBDetailedLog: TCheckBox
      Left = 20
      Top = 112
      Width = 113
      Height = 17
      Caption = 'Detailed text log'
      TabOrder = 3
    end
  end
  object GBUI: TGroupBox
    Left = 8
    Top = 335
    Width = 448
    Height = 70
    Caption = 'User interface'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    object Label8: TLabel
      Left = 16
      Top = 47
      Width = 105
      Height = 13
      Caption = 'Font size for treeview'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label9: TLabel
      Left = 270
      Top = 47
      Width = 50
      Height = 13
      Caption = '(percents)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object EFontSize: TEdit
      Left = 204
      Top = 42
      Width = 60
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      NumbersOnly = True
      ParentFont = False
      TabOrder = 1
    end
    object CBLeaveWindowsAfterBreak: TCheckBox
      Left = 16
      Top = 23
      Width = 297
      Height = 17
      Caption = 'Leave nested open windows in case of user break'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
    end
  end
  object GBDefaultXML: TGroupBox
    Left = 8
    Top = 407
    Width = 448
    Height = 89
    Caption = 'Default control module file (XML)'
    TabOrder = 3
    object Label10: TLabel
      Left = 16
      Top = 41
      Width = 98
      Height = 13
      Caption = 'Filename (with path)'
    end
    object EDefaultModule: TEdit
      Left = 39
      Top = 60
      Width = 378
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
    object CBExecuteDefaultModule: TCheckBox
      Left = 20
      Top = 18
      Width = 113
      Height = 17
      Caption = 'Execute file'
      TabOrder = 0
    end
    object BChooseFile: TButton
      Left = 416
      Top = 60
      Width = 21
      Height = 21
      Caption = '...'
      TabOrder = 2
      OnClick = BChooseFileClick
    end
  end
  object GroupBox1: TGroupBox
    Left = 9
    Top = 497
    Width = 448
    Height = 54
    Caption = 'Report nonzero exitcodes as errors'
    TabOrder = 6
    object CBCommandExitCodeErrors: TCheckBox
      Left = 15
      Top = 19
      Width = 181
      Height = 17
      Caption = 'For cmd.exe launches'
      TabOrder = 0
    end
    object CBAppsExitCodesErrors: TCheckBox
      Left = 202
      Top = 19
      Width = 234
      Height = 17
      Caption = 'For other executables (except Office apps)'
      TabOrder = 1
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'XML files (*.xml)|*.xml'
    Left = 272
    Top = 247
  end
end
