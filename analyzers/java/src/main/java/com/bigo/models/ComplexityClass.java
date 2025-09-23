package com.bigo.models;

public enum ComplexityClass {
    O_1("O(1)"),
    O_LOG_N("O(log n)"),
    O_SQRT_N("O(√n)"),
    O_N("O(n)"),
    O_N_LOG_N("O(n log n)"),
    O_N_SQUARED("O(n²)"),
    O_N_CUBED("O(n³)"),
    O_2_N("O(2^n)"),
    O_N_FACTORIAL("O(n!)");
    
    private final String notation;
    
    ComplexityClass(String notation) {
        this.notation = notation;
    }
    
    @Override
    public String toString() {
        return notation;
    }
}