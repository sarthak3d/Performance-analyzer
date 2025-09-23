package com.bigo;

import com.bigo.analyzer.JavaAnalyzer;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.stream.Collectors;

public class Main {
    public static void main(String[] args) {
        try {
            // Read JSON input from stdin
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
            String jsonInput = reader.lines().collect(Collectors.joining("\n"));
            
            // Parse input
            Gson gson = new Gson();
            AnalysisRequest request = gson.fromJson(jsonInput, AnalysisRequest.class);
            
            // Analyze code
            JavaAnalyzer analyzer = new JavaAnalyzer();
            var result = analyzer.analyze(request.code);
            
            // Output result as JSON
            Gson outputGson = new GsonBuilder().setPrettyPrinting().create();
            String jsonOutput = outputGson.toJson(result);
            System.out.println(jsonOutput);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }
    
    static class AnalysisRequest {
        String code;
        String language;
        String fileName;
    }
}