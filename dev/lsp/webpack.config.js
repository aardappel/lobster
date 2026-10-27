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
        lobster_lsp: './src/server.ts',
        lobsterfmt: './src/format/main.ts',
    },
    output: {
        filename: '[name].js',
        path: path.join(__dirname, 'webpack-out'),
        libraryTarget: "commonjs",
    }
};