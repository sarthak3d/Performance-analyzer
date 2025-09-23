"""
Python Code Complexity Analyzer
AST-based analysis for Python code complexity detection.
"""

import ast
from typing import Optional, Set, Dict, List
from enum import Enum
from dataclasses import dataclass
from loguru import logger


class ComplexityClass(str, Enum):
    """Complexity class enumeration."""
    O_1 = "O(1)"
    O_LOG_N = "O(log n)"
    O_N = "O(n)"
    O_N_LOG_N = "O(n log n)"
    O_N_SQUARED = "O(n²)"
    O_N_CUBED = "O(n³)"
    O_2_N = "O(2^n)"
    O_N_FACTORIAL = "O(n!)"


@dataclass
class ComplexityResult:
    """Result of complexity analysis."""
    time_complexity: ComplexityClass
    space_complexity: ComplexityClass
    best_case_time: Optional[ComplexityClass] = None
    average_case_time: Optional[ComplexityClass] = None
    worst_case_time: Optional[ComplexityClass] = None
    best_case_space: Optional[ComplexityClass] = None
    average_case_space: Optional[ComplexityClass] = None
    worst_case_space: Optional[ComplexityClass] = None
    confidence: float = 0.0
    explanation: str = ""
    patterns: list = None
    recursive: bool = False
    has_loops: bool = False
    loop_depth: int = 0
    suggestions: list = None
    
    def __post_init__(self):
        if self.patterns is None:
            self.patterns = []
        if self.suggestions is None:
            self.suggestions = []
        
        # Set default cases
        if self.worst_case_time is None:
            self.worst_case_time = self.time_complexity
        if self.average_case_time is None:
            self.average_case_time = self.time_complexity
        if self.best_case_time is None:
            self.best_case_time = self.time_complexity
        if self.worst_case_space is None:
            self.worst_case_space = self.space_complexity
        if self.average_case_space is None:
            self.average_case_space = self.space_complexity
        if self.best_case_space is None:
            self.best_case_space = self.space_complexity


class PythonComplexityAnalyzer(ast.NodeVisitor):
    """AST-based complexity analyzer for Python code."""
    
    def __init__(self):
        self.reset()
    
    def reset(self):
        """Reset analyzer state."""
        self.loop_depth = 0
        self.max_loop_depth = 0
        self.recursive_calls = []
        self.function_calls = []
        self.patterns_found = []
        self.current_function = None
        self.complexity_factors = []
        self.function_definitions = {}  # Store function definitions
        self.recursive_call_counts = {}  # Count recursive calls per function
        self.logarithmic_patterns = False
        self.while_loops = []  # Store while loop info
        self.divide_conquer_pattern = False
    
    def analyze(self, code: str, **options) -> ComplexityResult:
        """Analyze Python code and determine its complexity."""
        self.reset()
        
        try:
            tree = ast.parse(code)
            
            # First pass: collect function definitions
            for node in ast.walk(tree):
                if isinstance(node, ast.FunctionDef):
                    self.function_definitions[node.name] = node
            
            # Second pass: analyze
            self.visit(tree)
            
            time_complexity = self._calculate_time_complexity()
            space_complexity = self._calculate_space_complexity()
            confidence = self._calculate_confidence()
            explanation = self._generate_explanation()
            
            result = ComplexityResult(
                time_complexity=time_complexity,
                space_complexity=space_complexity,
                confidence=confidence,
                explanation=explanation,
                patterns=self.patterns_found,
                recursive=bool(self.recursive_calls),
                has_loops=self.max_loop_depth > 0,
                loop_depth=self.max_loop_depth
            )
            
            result.suggestions = self._generate_suggestions(result)
            return result
            
        except SyntaxError as e:
            logger.error(f"Syntax error: {e}")
            raise ValueError(f"Invalid Python syntax: {e}")
    
    def visit_FunctionDef(self, node: ast.FunctionDef):
        """Visit function definition."""
        old_function = self.current_function
        self.current_function = node.name
        self.recursive_call_counts[node.name] = 0
        
        # Check for divide and conquer pattern (functions like merge, merge_sort)
        if 'merge' in node.name.lower() or 'sort' in node.name.lower():
            if self._check_divide_conquer_pattern(node):
                self.divide_conquer_pattern = True
                self.patterns_found.append("divide_conquer")
        
        self.generic_visit(node)
        self.current_function = old_function
    
    def visit_For(self, node: ast.For):
        """Visit for loop."""
        self.loop_depth += 1
        self.max_loop_depth = max(self.max_loop_depth, self.loop_depth)
        
        # Check if it's a range loop with power of 2 steps
        if isinstance(node.iter, ast.Call):
            func_name = self._get_function_name(node.iter)
            if func_name == 'range' and len(node.iter.args) >= 3:
                # Check for logarithmic step pattern
                step = node.iter.args[2] if len(node.iter.args) > 2 else None
                if step and self._is_logarithmic_step(step):
                    self.logarithmic_patterns = True
                    self.patterns_found.append("logarithmic_loop")
        
        self.generic_visit(node)
        self.loop_depth -= 1
    
    def visit_While(self, node: ast.While):
        """Visit while loop."""
        self.loop_depth += 1
        self.max_loop_depth = max(self.max_loop_depth, self.loop_depth)
        
        # Store while loop for analysis
        self.while_loops.append(node)
        
        # Check for logarithmic patterns in while condition and body
        if self._check_logarithmic_while(node):
            self.logarithmic_patterns = True
            self.patterns_found.append("logarithmic_while")
        
        self.generic_visit(node)
        self.loop_depth -= 1
    
    def visit_Call(self, node: ast.Call):
        """Visit function call."""
        func_name = self._get_function_name(node)
        if func_name:
            self.function_calls.append(func_name)
            
            # Check for recursive calls
            if func_name == self.current_function:
                self.recursive_calls.append(func_name)
                self.recursive_call_counts[func_name] = self.recursive_call_counts.get(func_name, 0) + 1
                if "recursion" not in self.patterns_found:
                    self.patterns_found.append("recursion")
            
            # Check for sorting functions
            if func_name in ['sorted', 'sort']:
                self.patterns_found.append("sorting")
                self.complexity_factors.append("n_log_n")
        
        self.generic_visit(node)
    
    def visit_BinOp(self, node: ast.BinOp):
        """Visit binary operation."""
        # Check for division operations that might indicate logarithmic behavior
        if isinstance(node.op, ast.FloorDiv) and self.loop_depth > 0:
            # Check if we're dividing by a constant (like 2)
            if isinstance(node.right, ast.Constant) and node.right.value > 1:
                self.complexity_factors.append("potential_logarithmic")
        
        self.generic_visit(node)
    
    def visit_AugAssign(self, node: ast.AugAssign):
        """Visit augmented assignment (like //=, *=)."""
        if self.loop_depth > 0:
            # Check for n //= 2 or similar patterns
            if isinstance(node.op, ast.FloorDiv):
                if isinstance(node.value, ast.Constant) and node.value.value > 1:
                    self.logarithmic_patterns = True
                    self.patterns_found.append("logarithmic_reduction")
            # Check for n *= 2 patterns
            elif isinstance(node.op, ast.Mult):
                if isinstance(node.value, ast.Constant) and node.value.value > 1:
                    self.logarithmic_patterns = True
                    self.patterns_found.append("logarithmic_growth")
        
        self.generic_visit(node)
    
    def _check_logarithmic_while(self, node: ast.While) -> bool:
        """Check if a while loop has logarithmic characteristics."""
        # Check the loop body for division or multiplication patterns
        for body_node in ast.walk(node):
            if isinstance(body_node, ast.AugAssign):
                # n //= 2, n /= 2 patterns
                if isinstance(body_node.op, (ast.FloorDiv, ast.Div)):
                    if isinstance(body_node.value, ast.Constant):
                        if body_node.value.value > 1:
                            return True
                # n *= 2 patterns (also logarithmic when counting iterations)
                elif isinstance(body_node.op, ast.Mult):
                    if isinstance(body_node.value, ast.Constant):
                        if body_node.value.value > 1:
                            return True
            
            # Check for n = n // 2 patterns
            elif isinstance(body_node, ast.Assign):
                if isinstance(body_node.value, ast.BinOp):
                    if isinstance(body_node.value.op, (ast.FloorDiv, ast.Div)):
                        if isinstance(body_node.value.right, ast.Constant):
                            if body_node.value.right.value > 1:
                                return True
        
        # Check condition for > 1 or > 0 patterns commonly used with logarithmic loops
        if isinstance(node.test, ast.Compare):
            for op in node.test.ops:
                if isinstance(op, (ast.Gt, ast.GtE)):
                    # This could be while n > 1 or while n > 0
                    return any('logarithmic' in factor for factor in self.complexity_factors)
        
        return False
    
    def _check_divide_conquer_pattern(self, func_node: ast.FunctionDef) -> bool:
        """Check if a function follows divide and conquer pattern."""
        recursive_calls = 0
        has_mid_calculation = False
        has_array_slicing = False
        
        for node in ast.walk(func_node):
            # Check for recursive calls
            if isinstance(node, ast.Call):
                func_name = self._get_function_name(node)
                if func_name == func_node.name:
                    recursive_calls += 1
            
            # Check for mid calculation (common in divide & conquer)
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and 'mid' in target.id.lower():
                        has_mid_calculation = True
            
            # Check for array slicing (arr[:mid], arr[mid:])
            if isinstance(node, ast.Subscript):
                if isinstance(node.slice, ast.Slice):
                    has_array_slicing = True
        
        # Divide and conquer typically has 2 recursive calls with array splitting
        return recursive_calls >= 2 and (has_mid_calculation or has_array_slicing)
    
    def _count_recursive_calls_in_function(self, func_name: str) -> int:
        """Count the number of recursive calls in a single execution path."""
        if func_name not in self.function_definitions:
            return 0
        
        func_node = self.function_definitions[func_name]
        recursive_count = 0
        
        # Count direct recursive calls not in loops
        for node in ast.walk(func_node):
            if isinstance(node, ast.Call):
                call_name = self._get_function_name(node)
                if call_name == func_name:
                    # Check if this call is not inside a loop
                    if not self._is_inside_loop(node, func_node):
                        recursive_count += 1
        
        return recursive_count
    
    def _is_inside_loop(self, node: ast.AST, func_node: ast.FunctionDef) -> bool:
        """Check if a node is inside a loop."""
        for parent in ast.walk(func_node):
            if isinstance(parent, (ast.For, ast.While)):
                for child in ast.walk(parent):
                    if child == node:
                        return True
        return False
    
    def _is_logarithmic_step(self, step_node: ast.AST) -> bool:
        """Check if a step value indicates logarithmic behavior."""
        # Check for multiplication or exponential steps
        if isinstance(step_node, ast.BinOp):
            if isinstance(step_node.op, (ast.Mult, ast.Pow)):
                return True
        return False
    
    def _get_function_name(self, node: ast.Call) -> Optional[str]:
        """Extract function name from call."""
        if isinstance(node.func, ast.Name):
            return node.func.id
        elif isinstance(node.func, ast.Attribute):
            return node.func.attr
        return None
    
    def _calculate_time_complexity(self) -> ComplexityClass:
        """Calculate time complexity."""
        # Check for sorting first
        if "sorting" in self.patterns_found:
            if self.max_loop_depth > 0:
                return ComplexityClass.O_N_SQUARED
            return ComplexityClass.O_N_LOG_N
        
        # Check for divide and conquer (like merge sort)
        if self.divide_conquer_pattern and self.recursive_calls:
            return ComplexityClass.O_N_LOG_N
        
        # Check for logarithmic patterns
        if self.logarithmic_patterns or "logarithmic_while" in self.patterns_found:
            if self.max_loop_depth > 1:
                # Logarithmic inside another loop
                return ComplexityClass.O_N_LOG_N
            return ComplexityClass.O_LOG_N
        
        # Check for exponential recursion (like fibonacci)
        if self.recursive_calls:
            # Count recursive calls per function
            for func_name in set(self.recursive_calls):
                recursive_count = self._count_recursive_calls_in_function(func_name)
                if recursive_count >= 2:
                    # Multiple recursive calls (like fibonacci)
                    return ComplexityClass.O_2_N
            
            # Single recursive call
            if self.max_loop_depth == 0:
                return ComplexityClass.O_N
            else:
                return ComplexityClass.O_N_SQUARED
        
        # Based on loop depth
        if self.max_loop_depth == 0:
            return ComplexityClass.O_1
        elif self.max_loop_depth == 1:
            return ComplexityClass.O_N
        elif self.max_loop_depth == 2:
            return ComplexityClass.O_N_SQUARED
        elif self.max_loop_depth == 3:
            return ComplexityClass.O_N_CUBED
        else:
            return ComplexityClass.O_2_N
    
    def _calculate_space_complexity(self) -> ComplexityClass:
        """Calculate space complexity."""
        # Recursive calls use stack space
        if self.recursive_calls:
            if self.divide_conquer_pattern:
                # Divide and conquer typically O(log n) stack space
                return ComplexityClass.O_LOG_N
            return ComplexityClass.O_N
        
        # Check for array/list creation in loops
        if self.max_loop_depth >= 2:
            return ComplexityClass.O_N_SQUARED
        elif self.max_loop_depth == 1:
            return ComplexityClass.O_N
        
        return ComplexityClass.O_1
    
    def _calculate_confidence(self) -> float:
        """Calculate confidence score."""
        confidence = 0.5
        
        if self.max_loop_depth > 0:
            confidence += 0.2
        if self.patterns_found:
            confidence += 0.2
        if self.recursive_calls:
            confidence += 0.1
        if self.logarithmic_patterns:
            confidence += 0.1
        if self.divide_conquer_pattern:
            confidence += 0.1
        
        return min(confidence, 0.95)
    
    def _generate_explanation(self) -> str:
        """Generate explanation."""
        explanations = []
        
        if self.divide_conquer_pattern:
            explanations.append("Divide and conquer pattern detected")
        
        if self.logarithmic_patterns:
            explanations.append("Logarithmic reduction pattern detected")
        
        if self.max_loop_depth > 0:
            explanations.append(f"Found {self.max_loop_depth} nested loop(s)")
        
        if self.recursive_calls:
            unique_recursive = set(self.recursive_calls)
            for func in unique_recursive:
                count = self._count_recursive_calls_in_function(func)
                if count >= 2:
                    explanations.append(f"Multiple recursive calls detected ({count} per execution)")
                else:
                    explanations.append("Single recursive call detected")
        
        if "sorting" in self.patterns_found:
            explanations.append("Uses sorting")
        
        return ". ".join(explanations) if explanations else "Sequential operations"
    
    def _generate_suggestions(self, result: ComplexityResult) -> list:
        """Generate optimization suggestions."""
        suggestions = []
        
        if result.time_complexity == ComplexityClass.O_2_N and result.recursive:
            suggestions.append("Consider using memoization or dynamic programming for exponential recursion")
        
        if result.time_complexity in [ComplexityClass.O_N_SQUARED, ComplexityClass.O_N_CUBED]:
            suggestions.append("Consider optimizing nested loops")
        
        if self.divide_conquer_pattern and result.space_complexity == ComplexityClass.O_N:
            suggestions.append("Divide and conquer implementation detected - space complexity is from recursion stack")
        
        return suggestions