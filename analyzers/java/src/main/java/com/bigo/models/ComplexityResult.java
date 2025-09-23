package com.bigo.models;

import java.util.ArrayList;
import java.util.List;

public class ComplexityResult {
    private String timeComplexity;
    private String spaceComplexity;
    private String bestCaseTime;
    private String averageCaseTime;
    private String worstCaseTime;
    private String bestCaseSpace;
    private String averageCaseSpace;
    private String worstCaseSpace;
    private double confidence;
    private String explanation;
    private List<String> patterns = new ArrayList<>();
    private List<String> suggestions = new ArrayList<>();
    private boolean recursive;
    private boolean hasLoops;
    private int loopDepth;
    
    // Getters and setters
    public String getTimeComplexity() { return timeComplexity; }
    public void setTimeComplexity(String timeComplexity) { 
        this.timeComplexity = timeComplexity; 
    }
    
    public String getSpaceComplexity() { return spaceComplexity; }
    public void setSpaceComplexity(String spaceComplexity) { 
        this.spaceComplexity = spaceComplexity; 
    }
    
    public String getBestCaseTime() { return bestCaseTime; }
    public void setBestCaseTime(String bestCaseTime) { 
        this.bestCaseTime = bestCaseTime; 
    }
    
    public String getAverageCaseTime() { return averageCaseTime; }
    public void setAverageCaseTime(String averageCaseTime) { 
        this.averageCaseTime = averageCaseTime; 
    }
    
    public String getWorstCaseTime() { return worstCaseTime; }
    public void setWorstCaseTime(String worstCaseTime) { 
        this.worstCaseTime = worstCaseTime; 
    }
    
    public String getBestCaseSpace() { return bestCaseSpace; }
    public void setBestCaseSpace(String bestCaseSpace) { 
        this.bestCaseSpace = bestCaseSpace; 
    }
    
    public String getAverageCaseSpace() { return averageCaseSpace; }
    public void setAverageCaseSpace(String averageCaseSpace) { 
        this.averageCaseSpace = averageCaseSpace; 
    }
    
    public String getWorstCaseSpace() { return worstCaseSpace; }
    public void setWorstCaseSpace(String worstCaseSpace) { 
        this.worstCaseSpace = worstCaseSpace; 
    }
    
    public double getConfidence() { return confidence; }
    public void setConfidence(double confidence) { 
        this.confidence = confidence; 
    }
    
    public String getExplanation() { return explanation; }
    public void setExplanation(String explanation) { 
        this.explanation = explanation; 
    }
    
    public List<String> getPatterns() { return patterns; }
    public void setPatterns(List<String> patterns) { 
        this.patterns = patterns; 
    }
    
    public List<String> getSuggestions() { return suggestions; }
    public void setSuggestions(List<String> suggestions) { 
        this.suggestions = suggestions; 
    }
    
    public boolean isRecursive() { return recursive; }
    public void setRecursive(boolean recursive) { 
        this.recursive = recursive; 
    }
    
    public boolean isHasLoops() { return hasLoops; }
    public void setHasLoops(boolean hasLoops) { 
        this.hasLoops = hasLoops; 
    }
    
    public int getLoopDepth() { return loopDepth; }
    public void setLoopDepth(int loopDepth) { 
        this.loopDepth = loopDepth; 
    }
}