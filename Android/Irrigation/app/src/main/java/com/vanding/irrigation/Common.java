package com.vanding.irrigation;

public class Common {
    public static double RoundToNearestNiceNumber(double val, boolean roundup)
    {
        int negated = 1;
        if (val <= 0) {
            val = -val;
            negated = -1;
            roundup = !roundup;
        }
        double nice = Math.pow(10, Math.ceil(Math.log10(val))); // get the first larger power of 10
        if (roundup) {
            if (val < 0.25 * nice)       nice = 0.25 * nice;
            else if (val < 0.5 * nice)   nice = 0.5 * nice;
            else                         nice = 1.0 * nice;
        } else {
            if (val < 0.25 * nice)       nice = 0.1 * nice;
            else if (val < 0.5 * nice)   nice = 0.25 * nice;
            else                         nice = 0.5 * nice;
        }
        return nice * negated;
    }

    public static long RoundUpToNearestNiceNumberOLD(long in) {
        double x = 1.1 * in;
        return (long) x;
    }

}
