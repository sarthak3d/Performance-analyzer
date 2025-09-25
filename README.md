Analyzer Configration File Format
'''
{
  "language_name": {
    "enabled": true,
    "timeout": 30,
    "max_file_size": 10485760,
    "file_extension": "ext",
    "in_process": false,
    "command": ["command", "arg1", "arg2"],
    // OR for in-process analyzers:
    "in_process": true,
    "analyzer_module": "pathto analyzer AnalyzerClass"
  }
}
'''
'''
analyzers/c/
├── include/
│   ├── analyzer.h
│   ├── ast_visitor.h
│   ├── complexity_calculator.h
│   ├── pattern_detector.h
│   ├── json_handler.h
│   └── types.h
├── src/
│   ├── main.c
│   ├── analyzer.c
│   ├── ast_visitor.c
│   ├── complexity_calculator.c
│   ├── pattern_detector.c
│   └── json_handler.c
└── CMakeLists.txt
'''