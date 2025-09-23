#!/usr/bin/env node

const readline = require('readline');
const { JavaScriptAnalyzer } = require('./analyzer');

// Read JSON input from stdin
async function readInput() {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        terminal: false
    });

    let input = '';
    
    for await (const line of rl) {
        input += line + '\n';
    }
    
    return input;
}

async function main() {
    try {
        // Read input
        const input = await readInput();
        const request = JSON.parse(input);
        
        // Analyze code
        const analyzer = new JavaScriptAnalyzer();
        const result = await analyzer.analyze(request.code);
        
        // Output result as JSON
        console.log(JSON.stringify(result, null, 2));
        
    } catch (error) {
        console.error('Error:', error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    main();
}