"""
Complexity class definitions.
"""

from enum import Enum


class ComplexityClass(str, Enum):
    """Big O complexity classes."""
    O_1 = "O(1)"
    O_LOG_N = "O(log n)"
    O_SQRT_N = "O(√n)"
    O_N = "O(n)"
    O_N_LOG_N = "O(n log n)"
    O_N_SQUARED = "O(n²)"
    O_N_CUBED = "O(n³)"
    O_2_N = "O(2^n)"
    O_N_FACTORIAL = "O(n!)"
    UNKNOWN = "O(?)"
    
    @classmethod
    def from_string(cls, value: str) -> "ComplexityClass":
        """Create ComplexityClass from string."""
        for member in cls:
            if member.value == value:
                return member
        return cls.UNKNOWN