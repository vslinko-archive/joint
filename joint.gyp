{
    "targets": [
        {
            "target_name": "joint",
            "type": "executable",
            "sources": [
                "src/joint.c",
                "src/source_file.c",
                "src/string.c",
                "src/tokenizer.c",
                "src/parser.c"
            ],
            "conditions": [
                ["OS=='mac'", {
                    "include_dirs": [
                        "/usr/local/include"
                    ],
                    "link_settings": {
                        "libraries": [
                            "/usr/local/lib/libargp.a"
                        ]
                    }
                }]
            ]
        }
    ]
}
