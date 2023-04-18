'use strict';

const path = require('path');

module.exports = {
    mode: 'none',
    target: 'node',
    node: {
        __dirname: false
    },
    resolve: {
        mainFields: ['module', 'main'],
        extensions: ['.ts','.js']
    },
    module: {
        rules: [{
            test: /\.ts$/,
            exclude: /node_modules/,
            use: [{
                // configure TypeScript loader:
                loader: 'ts-loader',
                options: {
                    compilerOptions: {
                        "sourceMap": true,
                    }
                }
            }]
        }]
    },
    externals: {
        'vscode': 'commonjs vscode',
    },
    devtool: 'source-map',
	context: path.join(__dirname),
    entry: {
        extension: './src/server.ts',
    },
    output: {
        filename: 'lobster_lsp.js',
        path: path.join(__dirname, 'webpack-out'),
        libraryTarget: "commonjs",
    }
};