BEGIN { PI=M_PI=3.14159265358979324; BIGNUM=1*1e30; for(i=0; i<256; i++) tree_insert("ASCII", sprintf("%c",i), i) } 
function ASSERT(cond,str) { if(!cond) { s=sprintf("ASSERTION failure, line %d of input file %s: %s.\nInput line was:\n<%s>\n", FNR,FILENAME,str,$0); print s >"/dev/stderr"; exit 1 }  } 
function WARN(cond,str,verbose) { if(!cond) { s=sprintf("WARNING: line %d of input file %s: %s",FNR,FILENAME,str); if(verbose) s=s sprintf("\nInput line was:\n<%s>\n", $0); print s >"/dev/stderr" }  } 
function ABS(x) { return x<0?-x:x } 
function SIGN(x) { return x==0?0:x/ABS(x) } 
function MAX(x,y) { return x>y?x:y } 
function MIN(x,y) { return x<y?x:y } 
function nul(s) {  } # do nothing; useful for holding temp strings in code on the command line.
# The default srand() uses time-of-day, which only changes once per second. Not good enough for paraell runs.
function GetFancySeed(           seed,hostname,n,h,i) { 
    seed = systime()+PROCINFO["gid"]+PROCINFO["uid"]+PROCINFO["pgrpid"]+PROCINFO["ppid"]+PROCINFO["pid"]; 
    "hostname"|getline hostname; n=length(hostname); for(i=1; i<=n; i++) seed += query_tree("ASCII", substr(hostname,i,1)); 
    return seed; 
 } 
function strip(s) { gsub("  *"," ",s); sub("  *$","",s); return s } 
function randsort(i1,v1,i2,v2) { return rand()-0.5 } # use this to have for loops go in random order
function Srand() { return srand(GetFancySeed());  } 
function RandInt(mean,radius) { return mean+2*radius*(rand()-0.5) } 
function ftos(f) { f=sprintf("%.3g",f); gsub("e[+]0","e+",f); return f } # remove leading 0s from exponent
function floor(x) { if(x>=0) return int(x); else return int(x)-1 } 
function ceil(x) { if(x==int(x)) return x; else return floor(x)+1 } 
function int2binary(i,l, _s) { if(i<0) return "nan"; _s=""; while(i) { _s=(i%2)""_s; i=int(i/2) } ; while(length(_s)<l)_s="0"_s; return _s } 
function Fatal(msg) { printf "FATAL ERROR: %s\n",msg >"/dev/stderr"; exit(1);  } 
function Warn(msg) { printf "Warning: %s\n",msg >"/dev/stderr" } 
function DotProd(u,v,    _dot,i) { while (tree_iters_remaining("u") > 0) { i = tree_next("u"); _dot += query_tree("u", i)*query_tree("v", i);  } return _dot;  } 
function NormDotProd(u,v) { return DotProd(u,v)/sqrt(DotProd(u,u)*DotProd(v,v));  } 
function VecDist(u,v,    i,dist2) { dist2=0; while (tree_iters_remaining("u") > 0) { i = tree_next("u"); dist2 += (query_tree("u", i)-query_tree("v", i))^2;  } return sqrt(dist2);  } 
function LogPSim(u,v,    i,res) { while (tree_iters_remaining("u") > 0) { i = tree_next("u"); if(query_tree("u", i)==query_tree("v", i)) res+=36; else res -= log(ABS(query_tree("u", i)-query_tree("v", i)));  } return res/length(u);  } 
function GeoMeanDist(u,v,    i,res) { while (tree_iters_remaining("u") > 0) { i = tree_next("u"); if(query_tree("u", i)==query_tree("v", i)) res+=log(1e-16); else res += log(ABS(query_tree("u", i)-query_tree("v", i)));  } return exp(res/length(u));  } 
function inarray(element,array,      i) { for(i=1; i<=length(array); i++) if(element==query_tree("array", i)) return 1; return 0 } 
function IsPrime(N,   i) { if(N<2) return 0; for(i=2; i<=sqrt(N); i++) if(N/i==int(N/i)) return 0; return 1 } 
function NSORT(a,ai,   i,NsortTc) { delete_tree("sortTb"); delete_tree("sortTc"); while (tree_iters_remaining("a") > 0) { i = tree_next("a"); tree_insert("sortTb", query_tree("a", i), i);  } NsortTc=asorti(sortTb,sortTc); for(i=1; i<=NsortTc; i++) tree_insert("ai", i, query_tree("sortTb", query_tree("sortTc", i))); return NsortTc } 
# Bubble Sort: assumes 1-indexed arrays!
function bsort(array,outindices,    N,i,j,temp) { 
    delete_tree("outindices"); delete_tree("iSortValue"); 
    N=0; while (tree_iters_remaining("array") > 0) { i = tree_next("array"); N++; tree_insert("outindices", N, i); tree_insert("iSortValue", N, query_tree("array", i)) } 
    for(i=2; i<=N; i++)      { 
 # Invariant: array from 1 .. i-1 is sorted, now bubble the next element into its place
 j=i; 
 while(j>1 && iSortValue[j] < iSortValue[j-1]) { 
     temp = query_tree("iSortValue", j-1); tree_insert("iSortValue", j-1, query_tree("iSortValue", j)); tree_insert("iSortValue", j, temp); 
     temp = query_tree("outindices", j-1); tree_insert("outindices", j-1, query_tree("outindices", j)); tree_insert("outindices", j, temp); 
     j--; 
  } 
     } 
    return N; 
 } 
function Factor(n,  i,s) { s=""; i=int(sqrt(n)); while(i>1) { while(n/i==int(n/i)) { s=s" "i; n/=i } i-- } ; return s } 
function asin(x) { return atan2(x, sqrt(1-x*x)) } 
function acos(x) { return atan2(sqrt(1-x*x), x) } 
function atan(x) { return atan2(x,1) } 
function sind(x) { return sin(x/180*PI) } 
function cosd(x) { return cos(x/180*PI) } 
function tand(x) { return tan(x/180*PI) } 
function asind(x) { return asin(x)/PI*180 } 
function acosd(x) { return acos(x)/PI*180 } 
function atand(x) { return atan(x)/PI*180 } 
# Given x, compute ln(1+x), using the Taylor series if necessary
function AccurateLog1(x,    absX,n,term,sum) { 
    # return log(1+x); # fuck it
    absX=ABS(x); 
    if(absX<1e-16) return x; # close to machine eps? it is just x
    if(absX>4e-6) return log(1+x); # built-in one is very good in this range
    ASSERT(x>=-0.5&&x<=1,"AccurateLog1("x") will not converge"); 
    if(tree_elem_exists("_memAccLog1", x)) return query_tree("_memAccLog1", x); 
    sum=0; 
    n=1; term=x; 
    delete_tree("_log1Terms"); 
    # This first loop is just to get the terms, not actually computing the true sum
    while(n==1 || ABS(term/sum)>1e-20) { sum+=ABS(term); tree_insert("_log1Terms", n++, term); term*=x/n } 
    # Now sum the terms smallest-to-largest, keeping the two signs separate
    for(i=n; i>0; i--) if(query_tree("_log1Terms", i)<0) tree_increment("_log1Terms", -1, query_tree("_log1Terms", i)); else tree_increment("_log1Terms", 0, query_tree("_log1Terms", i))
    sum = query_tree("_log1Terms", 0) + query_tree("_log1Terms", -1); 
    sum -= sum*sum; # I am not sure why, but this gives a MUCH better approximation???
    if(n>_nMaxAccLog1) { _nMaxAccLog1=n; 
 # printf "AccurateLog: memoize log(1+%.16g)=%.16g, nMax %d\n",x,sum,_nMaxAccLog1 >"/dev/fd/2"
     } 
    return (tree_insert("_memAccLog1", x, sum)); 
 } 
# Assuming S=a+b, but we only have log(a) and log(b), we want to compute log(S)=log(a+b)=log(a(1+b/a))=log(a)+log(1+b/a)
# And then we can call AccurateLog1(b/a), except we do not have b/a explicitly, but we can get it with Exp(log_b-log_a)
function LogSumLogs(log_a,log_b,    truth, approx) { 
    m=MIN(log_a,log_b)
    M=MAX(log_a,log_b)
    ASSERT(M>=m,"BUG: M is not greater than m in LogSumLogs"); 
    # return M+log(1+Exp(m-M))
    if(M-m > 37) return M; # m < M*machine_eps, so m will not change M.
    # fuck it
    approx = M+AccurateLog1(Exp(m-M))
    if(ABS(log_a)<700 && ABS(log_b) < 700) { 
 truth=log(exp(log_a)+exp(log_b)); 
 if(ABS((approx-truth)/truth)>1e-10) 
     printf "LogSumLogs badApprox: log_a %g log_b %g M %g m %g approx %g truth %g\n",log_a,log_b,M,m,approx,truth > "/dev/stderr"
     } 
    return approx
 } 
# Given the logarithm log(p) of an arbitrarily large or small number p, return a string
# looking like printf("%.Ng", p) printing p with N significant digits (just like printf).
function logPrint(logp,digits,   l10_p,intLog,offset,mantissa,fmt) { 
    if(!digits) digits=6; # same as the default for printf
    if(ABS(logp)<100) { 
 fmt = sprintf("%%.%dg",digits); 
 return sprintf(fmt, exp(logp)); 
     } 
    else fmt = sprintf("%%.%dge%%+d",digits); 
    l10_p = logp/log(10); 
    if(l10_p<0) offset = floor(l10_p); 
    intLog=int(l10_p-offset)
    mantissa=10^(l10_p-intLog-offset)
    return sprintf(fmt, mantissa, intLog+offset); 
 } 
function fact(k) { if(k<=0) return 1; else return k*fact(k-1) } 
function logFact(k) { if(tree_elem_exists("_memLogFact", k)) return query_tree("_memLogFact", k); 
    if(k<=0) return 0; else return (tree_insert("_memLogFact", k, log(k)+logFact(k-1))); 
 } 
function log2NumSimpleGraphs(n) { # log2(number of non-isomorphic graphs on n nodes), ie. number of bits needed to count them
    # see https://oeis.org/A000088 for comparison for n=0,.. 19
    return choose(n,2)-logFact(n)/log(2)# https://cw.fel.cvut.cz/b211/_media/courses/b4m33pal/lectures/isom_notes.pptx
 } 
function fact2(k) { if(k<=1) return 1; else return k*fact2(k-2) } 
function logFact2(k) { if(k<=1) return 0; else return log(k)+logFact2(k-2) } 
# see Reza expansion: (n k) = ((n-1) (k-1)) + ((n-1) k)
function choose(n,k,     r,i) { if(0<=k&&k<=n) { r=1; for(i=1; i<=k; i++) r*=(n-(k-i))/i;  } else { r=0; Warn("choose: ("n" choose "k") may not make sense; returning 0") } ; return r } 
function logChoose(n,k) { if(tree_elem_exists("_memLogChoose", n) && tree_elem_exists("_memLogChoose", n, k)) return query_tree("_memLogChoose", n, k); 
    if(n<k) return log(0); # remove this line if it causes unnecessary failures
    else ASSERT(0<=k && k <=n,"invalid logChoose("n","k")"); 
    return (tree_insert("_memLogChoose", n, k, logFact(n)-logFact(k)-logFact(n-k))); 
 } 
function logChooseClever(n,k,     r,i) { 
    ASSERT(0<=k&&k<=n,"impossible parameters to logChoose "n" "k)
    if(tree_elem_exists("_logChooseMemory", n) && tree_elem_exists("_logChooseMemory", n, k)) return query_tree("_logChooseMemory", n, k); 
    r=0; for(i=1; i<=k; i++) r+=log(n-(k-i))-log(i)
    tree_insert("_logChooseMemory", n, k, r)
    return r; 
 } 
function HalfGamma(k) { return sqrt(PI) *   fact2(k-2)/sqrt(2)^(k-1) } 
function logHalfGamma(k) { return log(sqrt(PI))+logFact2(k-2)-(k-1)*log(sqrt(2)) } 
function Gamma(x) { if(x==int(x)) return fact(x-1); if(2*x==int(2*x)) return HalfGamma(2*x); else ASSERT(0,"Gamma only for integers and half-integers") } 
function logGamma(x) { if(x==int(x)) return logFact(x-1); if(2*x==int(2*x)) return logHalfGamma(2*x); else ASSERT(0,"Gamma only for integers and half-integers") } 
function IncGamma(s,x) { 
    ASSERT(s==int(s)&&s>=1,"IncGamma(s="s",x="x"): s must be int>=1 for now"); 
    if(s==1) return Exp(-x)
    else return (s-1)*IncGamma(s-1,x) + x^(s-1)*Exp(-x)
 } 
function logIncGamma(s,x) { 
    ASSERT(s==int(s)&&s>=1,"logIncGamma: s must be int>=1 for now"); 
    if(s==1) return -x; 
    else { 
 ASSERT(x>0,"logIncGamma: x=" x " must be > 0"); 
 log_a = log(s-1)+logIncGamma(s-1,x)
 log_c = (s-1)*log(x)-x
 return LogSumLogs(log_a,log_c)
     } 
 } 
function BinarySearch(dir,array,z, n,i,L,R,m,x) { 
    ASSERT(dir==1 || dir==-1, "BinarySearch: direction must  be +/-1, not "dir); 
    n=length(array); 
    # for(i=1;i<=n;i++) ASSERT(i in array, "BinarySearch: "i" is not in array of length "n);
    if(dir*(z-query_tree("array", 1))< 0) return 0; 
    if(dir*(z-query_tree("array", n))>=0) return n; 
    L=1; R=n; 
    while(L < R) { 
        m = int((L + R) / 2); 
 # ASSERT(m>0 && m<=n, "BinarySearch: m "m" is out of bounds for n "n" L "L" R "R);
 # ASSERT(m in array,"oops, m is "m" out of n="n);
 x = query_tree("array", m); 
      if(dir*(x-z) < 0) L = m + 1
        else if(dir*(x-z) > 0) R = m - 1
        else return m
     } 
    # At this point, the value was not found, so return the m just below z
    m = int((L + R) / 2); 
    if(m>0 && m<=n) { 
 # ASSERT(m in array,"oops, m is "m" out of n="n);
 x = query_tree("array", m); 
 while(m>0 && dir*(array[m]-z) > 0) --m; 
     } 
    # printf "FOUND x %g at m %d from n %d L %d R %d\n", x,m,n,L,R
    return m; 
 } 
# Since gawk cannot pass arrays as parameters, we usurp the global array _Chi2_bins[*][*]. The first index of this array
# is NAME; for a fixed name, the second index is the bins, which are assumed to be equally probable.
function Chi2_stat(name,   bin,X2,avg) { ASSERT(tree_elem_exists("_Chi2_bins", name) && isarray(query_tree("_Chi2_bins", name)), "Chi2_Stat: query_tree("_Chi2_bins", "name") must be an array of your bin counts"); 
    tree_insert("_Chi2_n", name, 0); while (tree_iters_remaining("_Chi2_bins", name) > 0) { bin = tree_next("_Chi2_bins", name); tree_increment("_Chi2_n", name, query_tree("_Chi2_bins", name, bin));  } 
    avg = query_tree("_Chi2_n", name)/length(query_tree("_Chi2_bins", name)); 
    X2=0; while (tree_iters_remaining("_Chi2_bins", name) > 0) { bin = tree_next("_Chi2_bins", name); X2 += (query_tree("_Chi2_bins", name, bin)-avg)^2/avg;  } 
    return X2; 
 } 
function    Chi2_pair2(df,X2) { ASSERT(df%2==0,"Chi2_pair2 df "df" must be even"); return IncGamma(df/2,X2/2) /  Gamma(df/2) } 
function logChi2_pair2(df,X2) { ASSERT(df%2==0,"Chi2_pair2 df "df" must be even"); return logIncGamma(df/2,X2/2)-logGamma(df/2) } 
function    Chi2_pair (df,X2) { return df%2==0 ? Chi2_pair2(df,X2) : sqrt(Chi2_pair2(df-1, X2)*Chi2_pair2(df+1,X2)) } 
function logChi2_pair (df,X2) { return df%2==0 ? logChi2_pair2(df,X2) : (logChi2_pair2(df-1, X2)+logChi2_pair2(df+1,X2))/2 } 
function Chi2_tail_raw(df, x) { return Chi2_pair(df, x) } 
function    Chi2_tail(name) { return Chi2_pair(length(query_tree("_Chi2_bins", name)),Chi2_stat(name)) } 
function logChi2_tail(name) { return logChi2_pair(length(query_tree("_Chi2_bins", name)),Chi2_stat(name)) } 
function NumBits(n,    b) { b=0; while(n>0) { if(n%2==1) b++; n=int(n/2) } ; return b } 
function log2(n) { return log(n)/log(2) } 
function log10(n) { return log(n)/log(10) } 
# res1 is your variable, where the output set goes; it will be nuked and replaced with the set intersection of T1 and T2.
# Return value is the cardinality of the result.
function SetIntersect(res,T1,T2,
    g) { delete_tree("res"); if(length(T1)<length(T2)) { while (tree_iters_remaining("T1") > 0) { g = tree_next("T1"); if(tree_elem_exists("T2", g)) tree_insert("res", g, 1);  } 
             else { while (tree_iters_remaining("T2") > 0) { g = tree_next("T2"); if(tree_elem_exists("T1", g)) tree_insert("res", g, 1);  } ; return tree_length("res") } 
# same as above but for set union, and res is the result.
function SetUnion(res,T1,T2,
    g) { delete_tree("res"); while (tree_iters_remaining("T1") > 0) { g = tree_next("T1"); tree_insert("res", g, 1);  } while (tree_iters_remaining("T2") > 0) { g = tree_next("T2"); tree_insert("res", g, 1);  } return tree_length("res") } 
# cumulative add set T to res3
function SetCumulativeUnion(res3,T, g) { while (tree_iters_remaining("T") > 0) { g = tree_next("T"); tree_insert("res3", g, 1);  } 
function SetCopy(dest,src,   g) { delete_tree("dest"); while (tree_iters_remaining("src") > 0) { g = tree_next("src"); tree_insert("dest", g, 1);  } 
function Jaccard(T1,T2,   i,u) { SetIntersect(i,T1,T2); SetUnion(u,T1,T2); return tree_length("i)/length(u");  } 
# And now counting the info in an edge list. One way to view the info is simply the number of edges.
# Another is to view each nodes adjacency list as having log(n) bits for each of its neighbors.
# This then says that the amount of info is as follows: the end of each edge is listed twice (ie is in two neighbor lists),
# but only one is strictly needed. And each entry is log2(n) bits. So the total info is just log2(n)*numEdges.
# But this number is *way* bigger than the number of edges for all 2018 BioGRID networks, so clearly it is too high.
function netbits(n1,n2,     i,bits) { if(n2==0) n2=n1; bits=0; for(i=0; i<MIN(n1,n2); i++) bits+=log(MAX(n1,n2)-i)/log(2); return bits } 
function logb(b,x) { return log(x)/log(b) } 
function dtob(n,   s,sgn) { n=1*n; if(!n) return "0"; s=sgn=""; if(n<0) { sgn="-"; n=-n } ; while(n) { s=sprintf("%d%s",(n%2),s); n=int(n/2) } ; return sgn s } 
function btod(n) {  } 
# Queue functions: call QueueAlloc(name) to allocate a queue with name "name"; then Add(name) and Next(name) do the obvious.
function QueueAlloc(name) { tree_insert("_queueFirst", name, 1); tree_insert("_queueLast", name, 0); tree_insert("_queueVals", name, 1, 1); tree_remove("_queueVals", name, 1);  } 
function QueueDelloc(name) { tree_remove("_queueFirst", name); tree_remove("_queueLast", name); tree_remove("_queueVals", name) } 
function QueueLength(name) { return query_tree("_queueLast", name)-query_tree("_queueFirst", name)+1;  } 
function QueueAdd(name, val) { tree_insert("_queueVals", name, query_tree("++_queueLast", name), val);  } 
function QueueNext(name,	val) { 
    val = query_tree("_queueVals", name, query_tree("_queueFirst", name)); 
    tree_increment("delete _queueVals", name, query_tree("_queueFirst", name), 1); 
    return val; 
 } 
# if quantiles is true (anything nonzero or nonempty string), remember everyting so we can retrieve quantiles later.
function StatReset(name, quantiles) { 
    tree_insert("_statQuantiles", name, quantiles); 
    tree_insert("_statN", name, query_tree("_statSum", name) = query_tree("_statSum2", name) = 0); 
    tree_insert("_statMin", name, BIGNUM); tree_insert("_statMax", name, -BIGNUM); 
    tree_insert("_statmin", name, BIGNUM); 
 } 
function StatHistReset(name) { 
    tree_remove("_statHist", name); 
    tree_remove("_statHistN", name); 
    tree_remove("_statHistMin", name); 
    tree_remove("_statHistCDF", name); 
    tree_remove("_statHistCDFix", name); 
 } 
function StatHistAddSample(name, x) { 
    if(!(tree_elem_exists("_statHistMin", name))) tree_insert("_statHistMin", name, 1*BIGNUM); 
    x=1*x; 
    if(x < query_tree("_statHistMin", name)) tree_insert("_statHistMin", name, x); 
    tree_increment("_statHist", name, x, 1); 
    tree_increment("_statHistN", name, 1); 
 } 
function StatHistMakeCDF(name,    n,x,prevX,PMF,prevSort) { 
    tree_remove("_statHistCDF", name); tree_remove("_statHistCDFix", name); 
    prevX=-1*(BIGNUM); # very very negative number
    n=0; tree_insert("_statHistCDFix", name, 0, prevX); 
    prevSort = PROCINFO["sorted_in"]; 
    tree_insert("PROCINFO", "sorted_in", "@ind_num_asc"); # traverse the array in numerical ascending order by index (ie., x)
    while (tree_iters_remaining("_statHist", name) > 0) { x = tree_next("_statHist", name); x=1*x; # ensure it is a number
 tree_insert("_statHistCDFix", name, ++n, x); 
 ASSERT(x > prevX, "oops, StatHistMakeCDF found non-incrementing x: "prevX" to "x); 
 PMF = query_tree("_statHist", name, x)/(query_tree("_statHistN", name)); 
 tree_insert("_statHistCDF", name, x, query_tree("_statHistCDF", name, prevX) + PMF); 
 # printf "_statHistCDF[%s][%g]=%g\n", name, x, _statHistCDF[name][x] >"/dev/stderr";
 prevX = x; 
     } 
    tree_insert("PROCINFO", "sorted_in", prevSort); 
    # _statHistCDF[name][prevX] may be above 1 due to numerical error; give it some leeway here.
    ASSERT(query_tree("_statHistCDF", name, prevX)<1+1e-6/query_tree("_statHistN", name), "query_tree("_statHistCDF", "name", "prevX")-1="query_tree("_statHistCDF", name, prevX)-1" which is too far above 1"); 
    tree_insert("_statHistCDF", name, prevX, 1); 
    tree_remove("_statHistCDF", name, -1*BIGNUM); # remove the -infinity elements created above
    tree_remove("_statHistCDFix", name, 0); 
 } 
# return the m with x closest to z with x<=z
function StatHistBinarySearch(name,z,    i,n,L,R,m,x) { 
    n = tree_length("_statHistCDFix[name]"); 
    for(i=1; i<=n; i++) ASSERT(tree_elem_exists("_statHistCDFix", name, i), i" is tree_elem_exists("F_ix", not) out of "n); 
    if(z < query_tree("_statHistCDFix", name, 1)) return 0; 
    if(z >= query_tree("_statHistCDFix", name, n)) return n; 
    L=1; R=n; 
    while(L < R) { 
        m = int((L + R) / 2); 
 ASSERT(m>0 && m<=n, "m "m" is out of bounds for n "n" L "L" R "R); 
 ASSERT(tree_elem_exists("_statHistCDFix", name, m),"oops, m is "m" out of n="n); 
 x = 1*query_tree("_statHistCDFix", name, m); 
 ASSERT(x==0|| (tree_elem_exists("_statHistCDF", name, x)"); 
 if(x < z) L = m + 1
        else if(x > z) R = m - 1
        else return m
     } 
    # At this point, the value was not found, so return the m just below z
    m = int((L + R) / 2); 
    if(m>0 && m<=n) { 
 ASSERT(tree_elem_exists("_statHistCDFix", name, m),"oops, m is "m" out of n="n); 
 x = query_tree("_statHistCDFix", name, m); 
 ASSERT(tree_elem_exists("_statHistCDF", name, x)"); 
 while(m>0 && _statHistCDFix[name][m] > z) --m; 
     } 
    # printf "FOUND x %g at m %d from n %d L %d R %d\n", x,m,n,L,R
    return m; 
 } 
function StatHistInterpSearch(name,z,    frac,i,n,L,R,m,x) { 
    n = tree_length("_statHistCDFix[name]"); 
    for(i=1; i<=n; i++) ASSERT(tree_elem_exists("_statHistCDFix", name, i), i" is tree_elem_exists("F_ix", not) out of "n); 
    if(z < query_tree("_statHistCDFix", name, 1)) return 0; 
    if(z >= query_tree("_statHistCDFix", name, n)) return n; 
    L=1; R=n; frac=0.5; 
    while(L < R) { 
 ASSERT(0<=frac && frac<=1,"frac "frac" out of bounds"); 
        m = int(frac*(L + R)); 
 ASSERT(m>0 && m<=n, "m "m" is out of bounds for n "n" L "L" R "R); 
 ASSERT(tree_elem_exists("_statHistCDFix", name, m),"oops, m is "m" out of n="n); 
 x = 1*query_tree("_statHistCDFix", name, m); 
 ASSERT(x==0|| (tree_elem_exists("_statHistCDF", name, x)"); 
 if(x < z) L = m + 1
        else if(x > z) R = m - 1
        else return m
 if(query_tree("_statHistCDFix", name, R) == query_tree("_statHistCDFix", name, L)) frac=0.5; 
 else frac = (x-query_tree("_statHistCDFix", name, L))/(query_tree("_statHistCDFix", name, R)-query_tree("_statHistCDFix", name, L)); 
 if(frac<=0 || frac>=1) frac=0.5; 
     } 
    # At this point, the value was not found, so return the m just below z
    m = int((L + R) / 2); 
    if(m>0 && m<=n) { 
 ASSERT(tree_elem_exists("_statHistCDFix", name, m),"oops, m is "m" out of n="n); 
 x = query_tree("_statHistCDFix", name, m); 
 ASSERT(tree_elem_exists("_statHistCDF", name, x)"); 
 while(m>0 && _statHistCDFix[name][m] > z) --m; 
     } 
    # printf "FOUND x %g at m %d from n %d L %d R %d\n", x,m,n,L,R
    return m; 
 } 
# Return the value in [0,1] of the empirical CDF of [name]
function StatHistECDF(name,z,  n,x,prevX,frac,h1,h2,interp,prevSort,m) { 
    z=1*z; 
    ASSERT(tree_elem_exists("_statHist", name), "StatHistECDF: no such histogram "name); 
    if(!(tree_elem_exists("_statHistCDF", name))) StatHistMakeCDF(name); 
    if(z<=query_tree("_statHistMin", name)) return 0; 
    n = tree_length("_statHistCDFix[name]"); 
    m=StatHistBinarySearch(name,z); 
    # printf "z %g i %d x %g\n", z, m, _statHistCDF[name][_statHistCDFix[name][m]]
    # in the following, h1 and h2 are actually x values
    ASSERT(query_tree("_statN", name), "StatHistECDF: name \""name"\" has no samples"); 
    if(m<1) return 1/(query_tree("_statN", name)*query_tree("_statN", name)); # h1=-BIGNUM;
    else h1 = query_tree("_statHistCDFix", name, m); 
    if(m>=n) return 1-1/(query_tree("_statN", name)*query_tree("_statN", name)); # h2=BIGNUM;
    else h2 = query_tree("_statHistCDFix", name, m+1); 
    frac=(z-h1)/(h2-h1); 
    # Now convert the x values to histogram values
    interp = query_tree("_statHistCDF", name, h1)+frac*(query_tree("_statHistCDF", name, h2) - query_tree("_statHistCDF", name, h1)); 
    return interp; # ######## COMMENT OUT THIS LINE TO CHECK THIS VALUE AGAINST OLD CORRECT CODE BELOW
    prevSort = PROCINFO["sorted_in"]; 
    tree_insert("PROCINFO", "sorted_in", "@ind_num_asc"); 
    while (tree_iters_remaining("_statHistCDF", name) > 0) { x = tree_next("_statHistCDF", name); 
 if(1*x>z) { 
     if(m>0) ASSERT(h1==prevX,"m is "m" with x1 "h1" but new is "prevX); 
     if(m>1) ASSERT(h2==    x,"m is "m" with x2 "h2" but new is "x); 
     if(m>0 && m<=n) ASSERT(frac==(z-prevX)/(x-prevX), "frac disagreement "frac" vs "(z-prevX)/(x-prevX)); 
     h1 = query_tree("_statHistCDF", name, prevX); h2 = query_tree("_statHistCDF", name, x); 
     tree_insert("PROCINFO", "sorted_in", prevSort); 
     ASSERT(interp == h1+frac*(h2-h1), "interp disagreement "interp" vs "h1+frac*(h2-h1)); 
     return h1+frac*(h2-h1); 
  } 
 prevX=x; 
     } 
    tree_insert("PROCINFO", "sorted_in", prevSort); 
    return 1; 
 } 
# Return the K-S (Kolmogorov-Smirnnov) statistic: the maximum distance between the empirical CDFs of name1 and name2
# FIXME: time is O((n1+n2)^2) since we loop through every value of the histogram, calling ECDF which ALSO does the SAME loop
# It can be done in time O(n1+n2) if we are a bit more clever.
function KSstat(name1,name2,   x,maxD,diff,sign) { 
    ASSERT(isarray(query_tree("_statHist", name1)) && isarray(query_tree("_statHist", name2)), "KSstat needs stats with histograms"); 
    maxD=0; 
    StatHistMakeCDF(name1); 
    StatHistMakeCDF(name2); 
    prevX = query_tree("_statHistMin", name1); 
    prevSort = PROCINFO["sorted_in"]; 
    tree_insert("PROCINFO", "sorted_in", "@ind_num_asc"); 
    while (tree_iters_remaining("_statHistCDF", name1) > 0) { x = tree_next("_statHistCDF", name1); 
 diff = query_tree("_statHistCDF", name1, x) - StatHistECDF(name2, x); 
 if(ABS(diff) > ABS(maxD)) { sign=1; maxD = diff } 
     } 
    prevX = query_tree("_statHistMin", name2); 
    while (tree_iters_remaining("_statHistCDF", name2) > 0) { x = tree_next("_statHistCDF", name2); 
 diff = query_tree("_statHistCDF", name2, x) - StatHistECDF(name1, x); 
 if(ABS(diff) > ABS(maxD)) { sign=-1; maxD = diff } 
     } 
    tree_insert("PROCINFO", "sorted_in", prevSort); 
    return sign*maxD; 
 } 
function KSpvalue(ks_stat,n1,n2, C) { 
    C=ks_stat/sqrt((n1+n2)/(n1*n2)); 
    return 2*Exp(-2*C*C); 
 } 
function KStest(name1, name2) { return KSpvalue(KSstat(name1,name2), query_tree("_statHistN", name1), query_tree("_statHistN", name2));  } 
function StatQuantile(name,q,   i,which,where,oldWhere,prevSort) { 
    ASSERT(query_tree("_statQuantiles", name), "StatQuantile called on name "name", but query_tree("_statQuantiles", name) is <"query_tree("_statQuantiles", name)">"); 
    ASSERT(0<= q && q<=1, "StatQuantile called with quantile q="q" which is not in [0,1]"); 
    where=0; 
    which = int(q*query_tree("_statN", name)+0.5); 
    # print "StatQuantile called with q="q" on "_statN[name]" elements; which is set to "which
    prevSort = PROCINFO["sorted_in"]; 
    tree_insert("PROCINFO", "sorted_in", "@ind_num_asc"); # traverse history in numerical order of the indices.
    while (tree_iters_remaining("_statValue", name) > 0) { x = tree_next("_statValue", name); 
 oldWhere=where; 
 where += query_tree("_statValue", name, x); 
 if(oldWhere <= which && which <=where) return x; 
     } 
    tree_insert("PROCINFO", "sorted_in", prevSort); 
 } 
function StatMedian(name) { return StatQuantile(name,0.5);  } 
function StatLowerQuartile(name) { return StatQuantile(name,0.25);  } 
function StatUpperQuartile(name) { return StatQuantile(name,0.75);  } 
function CircBufReset(name) { 
    tree_remove("_cBuf", name); tree_remove("_cBufLen", name); tree_remove("_cBufIn", name); tree_remove("_cBufOut", name); tree_remove("_cBufUsed", name); 
 } 
function CircBufPut(name,len,x,   n) { 
    if(!(name in _cBuf)) { 
 ASSERT(len>0, "CircBufPut["name"] called first time with invalid length "len); 
 tree_insert("_cBufLen", name, len); 
 tree_insert("_cBufIn", name, query_tree("_cBufOut", name)=query_tree("_cBufUsed", name)=0); 
     } 
    if(len != query_tree("_cBufLen", name)) { # resize the buffer
 Warn("CircBufPut: resizing buf "name" from "query_tree("_cBufLen", name)" to "len); 
 while(_cBufUsed[name]>=len) CircBufGet(name); # discard extra elements if buffer is shrinking
 ASSERT(!isarray(query_tree("_cBuf", "newBuf"))); 
 CircBufReset("newBuf"); 
 while(_cBufUsed[name]>0) CircBufPut("newBuf",len,CircBufGet(name)); 
 n = asort(query_tree("_cBuf", "newBuf"), query_tree("_cBuf", name)); # a side-effect of asort is to copy _cBuf["newBuf"] to _cBuf[name]
 ASSERT(n<len, "asort copied wrong number of elements "n" in CircBufPut with new length "len); 
 tree_insert("_cBufLen", name, query_tree("_cBufLen", "newBuf")); 
 tree_insert("_cBufIn", name, query_tree("_cBufIn", "newBuf")); 
 tree_insert("_cBufOut", name, query_tree("_cBufOut", "newBuf")); 
 tree_insert("_cBufUsed", name, query_tree("_cBufUsed", "newBuf")); 
 tree_remove("_cBuf", "newBuf"); tree_remove("_cBufLen", "newBuf"); tree_remove("_cBufIn", "newBuf"); 
 tree_remove("_cBufOut", "newBuf"); tree_remove("_cBufUsed", "newBuf"); 
     } 
    if(query_tree("_cBufUsed", name) == query_tree("_cBufLen", name)) Fatal("CircBufPut["name"]: overflow! Buffer already has "_cBufLen[name]" elements!"); 
    tree_insert("_cBuf", name, query_tree("_cBufIn", name), x); 
    tree_insert("_cBufIn", name, (query_tree("_cBufIn", name) + 1) % query_tree("_cBufLen", name)); 
    tree_increment("_cBufUsed", name, 1); 
 } 
function CircBufPeek(name) { 
    if(query_tree("_cBufUsed", name) == 0) Fatal("CircBufPeek["name"]: nothing to see!"); 
    return query_tree("_cBuf", name, query_tree("_cBufOut", name)); 
 } 
function CircBufGet(name,    val) { 
    if(query_tree("_cBufUsed", name) == 0) Fatal("CircBufGet["name"]: underflow!"); 
    val = query_tree("_cBuf", name, query_tree("_cBufOut", name)); 
    tree_insert("_cBufOut", name, (query_tree("_cBufOut", name) + 1) % query_tree("_cBufLen", name)); 
    tree_decrement("_cBufUsed", name, 1); 
    return val; 
 } 
function CircBufMean(name,N,x,   oldN,i) { 
    oldN = query_tree("_cBufLen", name); 
    if((tree_elem_exists("_cBuf", name)) && query_tree("_cBufUsed", name) == query_tree("_cBufLen", name)) tree_decrement("_cSum", name, CircBufGet(name)); 
    CircBufPut(name,N,x); 
    if(N==oldN) tree_increment("_cSum", name, x); 
    else { # buffer changed length, recompute sum
 tree_insert("_cSum", name, 0); 
 while (tree_iters_remaining("_cBuf", name) > 0) { i = tree_next("_cBuf", name); tree_increment("_cSum", name, query_tree("_cBuf", name, i));  } 
     } 
    return query_tree("_cSum", name)/query_tree("_cBufUsed", name); 
 } 
function StatAddSample(name, x) { 
    if(1*query_tree("_statN", name)==0 && !query_tree("_statQuantiles", name)) StatReset(name); 
    tree_increment("_statN", name, 1); 
    tree_increment("_statSum", name, x); 
    tree_increment("_statSum2", name, x*x); 
    tree_insert("_statMin", name, MIN(query_tree("_statMin", name),x)); 
    if(x) tree_insert("_statmin", name, MIN(query_tree("_statmin", name),x)); 
    tree_insert("_statMax", name, MAX(query_tree("_statMax", name),x)); 
    if(query_tree("_statQuantiles", name)) tree_increment("_statValue", name, x, 1); 
 } 
function StatAddSampleWeighted(name, x, w) { 
    if(1*query_tree("_statN", name)==0) StatReset(name); 
    tree_increment("_statN", name, w); 
    tree_increment("_statNunWtd", name, 1); 
    tree_increment("_statSum", name, w*x); 
    tree_increment("_statSum2", name, w*x*x); 
    tree_insert("_statMin", name, MIN(query_tree("_statMin", name),x)); 
    if(x) tree_insert("_statmin", name, MIN(query_tree("_statmin", name),x)); 
    tree_insert("_statMax", name, MAX(query_tree("_statMax", name),x)); 
 } 
function StatMean(name) { 
    ASSERT(query_tree("_statN", name)>0, "StatMean: name \""name"\" has no samples"); 
    return query_tree("_statSum", name)/query_tree("_statN", name); 
 } 
function StatMin(name) { 
    return query_tree("_statMin", name); 
 } 
function Statmin(name) { 
    return query_tree("_statmin", name); 
 } 
function StatMax(name) { 
    return query_tree("_statMax", name); 
 } 
function StatVar(name) { 
    if(query_tree("_statN", name)<2) return 0; 
    return (query_tree("_statSum2", name) - query_tree("_statSum", name)*query_tree("_statSum", name)/query_tree("_statN", name)) / (query_tree("_statN", name)-1); 
 } 
function StatStdDev(name,     x) { 
    x=StatVar(name); if(x<0) return 0; 
    return sqrt(StatVar(name)); 
 } 
function StatN(name) { 
    return query_tree("_statN", name); 
 } 
function FmtLtr(v) { return (v==int(v) ? "d" : "g");  } 
function StatPrint(name,  n,mu,m,M,s,v,fmt) { 
    n = query_tree("_statN", name); mu=StatMean(name); m=StatMin(name); M=StatMax(name); s=StatStdDev(name); v=StatVar(name); 
    fmt="# %6d\tmean %9" FmtLtr(mu) "\tmin %9" FmtLtr(m) "\tmax %9" FmtLtr(M) "\tstdDev %9" FmtLtr(s) "\tvar %9" FmtLtr(v);
    return sprintf(fmt, query_tree("_statN", name), StatMean(name), StatMin(name), StatMax(name), StatStdDev(name), StatVar(name)); 
 } 
function Norm2(n,vec,      i,sum2)
 { 
    sum2 = 0; 
    for(i=0; i<n; i++) sum2 += query_tree("vec", i)*query_tree("vec", i); 
    return sqrt(sum2); 
 } 
function NormalPtoZ(quantile,    q,z1,n,d)
 { 
    # printf "NormalPtoZ input is %g\n", quantile
    q = quantile > 0.5 ? (1 - quantile) : quantile; 
    # printf "NormalPtoZ q is %g\n", q
    z1 = sqrt (-2.0 * log (q)); 
    n = (0.010328 * z1 + 0.802853) * z1 + 2.515517; 
    d = ((0.001308 * z1 + 0.189269) * z1 + 1.43278) * z1 + 1.0; 
    z1 -= n / d; 
    return (quantile > 0.5 ? -z1 : z1); 
 } 
function Exp(x) { 
    if(x < -745) return 5e-324
    else if(x > 707) return BIGNUM7; 
    else return exp(x); 
 } 
function NormalPhi(x,    arg)
 { 
    arg=-x*x/2; 
    if(arg<-723) return 1e-314; 
    return 0.39894228040143267794*Exp(arg)
 } 
function NormalDist(mu,sigma,x,  E) { E=(mu-x)^2/(2*sigma*sigma); return Exp(-E)/(sigma*2.506628274631000502415765284811) } 
function NormalZ2P(x,    b0,b1,b2,b3,b4,b5,t,paren)
 { 
    if(x<0) return 1-NormalZ2P(-x); 
    b0 = 0.2316419; b1 = 0.319381530; b2 = -0.356563782; b3 = 1.781477937; b4 = -1.821255978; b5 = 1.330274429; 
    t=1/(1+b0*x); 
    paren = t*(b1+t*(b2+t*(b3+t*(b4+t*b5)))); 
    return NormalPhi(x)*paren; 
 } 
function StatTDistPtoZ(quantile, freedom,        z1,z2,h,x,i)
 { 
    # printf "StatTDistPtoZ input is (%g,%d)\n",quantile,freedom;
    z1 = ABS(NormalPtoZ(quantile)); 
    z2 = z1 * z1; 
    tree_insert("h", 0, 0.25 * z1 * (z2 + 1.0)); 
    tree_insert("h", 1, 0.010416667 * z1 * ((5.0 * z2 + 16.0) * z2 + 3.0)); 
    tree_insert("h", 2, 0.002604167 * z1 * (((3.0 * z2 + 19.0) * z2 + 17.0) * z2 - 15.0)); 
    tree_insert("h", 3, z1 * ((((79.0 * z2 + 776.0) * z2 + 1482.0) * z2 - 1920.0) * z2 - 945.0)); 
    tree_modify("h", 3, "*0.000010851"); 
    x = 0.0; 
    for(i = 3; i >= 0; i--)     x = (x + query_tree("h", i)) / freedom; 
    z1 += x; 
    return (quantile > 0.5 ? -z1 : z1); 
 } 
function StatTDistZtoP(t,n,     z,y,b,a)
 { 
    if(t<0) return 1-StatTDistZtoP(-t,n); 
    z=1; t=t*t; y=t/n; b=1+y; 
    if(n>=20&&t<n||n>200) { 
 if(y>1e-6) y=log(b); 
 a=n-.5; b=48*a*a; y=a*y; 
 y=(((((-.4*y-3.3)*y-24)*y-85.5)/(.8*y*y+100+b)+y+3)/b+1)*sqrt(y); 
 return NormalPhi(-y); 
     } else { 
 a=y=sqrt(y); if(n==1) a=0; 
 n-=2; 
 while(n>1) { 
     a=a*(n-1)/(b*n)+y; 
     n=-2; 
  } 
 a=(n==0?a/sqrt(b):(atan2(y,1)+a/b)*0.63661977236); 
 return ABS(z-a)/2; 
     } 
 } 
function StatConfidenceInterval(name,conf)
 { 
    return StatTDistPtoZ((1-conf)/2, query_tree("_statN", name) - 1) * sqrt(StatVar(name) / query_tree("_statN", name)); 
 } 
function BinomialPMF(p,n,k) { return choose(n,k)*p^k*(1-p)^(n-k) } 
function logBinomialPMF(p,n,k) { return logChoose(n,k)+log(p)*k+log(1-p)*(n-k) } 
function BinomialCDF(p,n,k, i,sum) { sum=0; 
    # Sum terms smallest to largest, along the shortest path to the endpoints since PMF is symmetric.
    if(k>=n/2) for(i=n; i>=k; i--) sum+=BinomialPMF(p,n,i); 
    else       for(i=0; i<=k; i++) sum+=BinomialPMF(1-p,n,n-i); 
    return sum
 } 
function logBinomialCDF(p,n,k, i,logSum) { 
    # Sum terms smallest to largest, along the shortest path to the endpoints since PMF is symmetric.
    if(k>=n/2) { logSum=logBinomialPMF(  p,n,k); for(i=n; i> k; i--) logSum=LogSumLogs(logSum, logBinomialPMF(  p,n,i)) } 
    else { logSum=logBinomialPMF(1-p,n,n); for(i=1; i<=k; i++) logSum=LogSumLogs(logSum, logBinomialPMF(1-p,n,n-i)) } 
    return logSum
 } 
# The Poisson1_CDF is 1-CDF, and sums terms smallest to largest; near CDF=1 (ie., 1-CDF=0) it is accurate well below eps_mach.
function PoissonCDF(l,k, sum, term, i) { sum=term=1; for(i=1; i<=k; i++) term*=l/i; sum+=term } ; return sum*Exp(-l) } 
function PoissonPMF(l,k, r,i) { if(l>723) return NormalDist(l,sqrt(l),k); r=Exp(-l); for(i=k; i>0; i--) r*=l/i; return r } 
function LogPoissonPMF(l,k, r,i) { r=-l; for(i=k; i>0; i--) r+=log(l/i); return r } 
function Poisson1_CDF(l,k, i,sum,psum) { psum=-1; sum=0; for(i=k; psum!=sum; i++) psum=sum; sum+=PoissonPMF(l,i) } ; 
    if(sum==0 && k<l) return 1; # this means the numbers are so big the sum got zero but we got less than expected.
    else return sum
 } 
function LogPoisson1_CDF(l,k, i,sum,pmax,max) { pmax=2; max=-BIGNUM; for(i=k; pmax!=max; i++) pmax=max; max=MAX(max,LogPoissonPMF(l,i)) } ; 
    if(max==1 && k<l) return 0; # this means the numbers are so big the sum got zero but we got less than expected.
    else return max/.894
 } 
# Return the logarithm of the area under the tail of the Normal(0,1) distribution from -infinity up to z
function logPhi(z) { 
    # The first is derived by L hopitals rule, and returns the log of the tail of the normal distribution at z.
    # At a p-value of 0.01, the error is about 3%, and the percent error drops by a factor of about 1.5 per factor of 10
    # decrease in p-value: so it is about 1.3% at p=1e-3; 0.7% at 1e-4; 0.43% at 1e-5; 0.3% at 1e-6; 0.22% at 1e-7, etc.
    if(z<-1.40532) return -(log(sqrt(2*PI))+z*z/2+log(-z)); 
    # The one below is a quadratic fit in the range sigma [-1.4,1] (ie., p-values in the range about 0.05 up to .8),
    # and can be off by a few tens of percent but for p-values so large... do we really care?
    else if(z<1.00268) return (-0.976263+((z+1.40532)/2.408)^(0.75)*(-.0686+0.9763))*log(10)
    # Otherwise this is an excellent approximation for large z
    else return -NormalZ2P(z)
 } 
function Log10Poisson1_CDF(l,k, i,sum,psum) { return LogPoisson1_CDF(l,k, i,sum,psum)/2.302585092994046 } 
# Hypergeometric distribution: Given: total population N, K of which have desired property.
# What is the probability of exactly k successes in n draws, without replacement?
function HyperGeomPMF(k,n,K,N) { return Exp(logHyperGeomPMF(k,n,K,N)) } 
function logHyperGeomPMF(k,n,K,N) { 
    # print "logHyperGeomPMF",k,n,K,N
    return logChoose(K,k)+logChoose(N-K,n-k)-logChoose(N,n); 
    # return logChoose(n,k)+logChoose(N-n,K-k)-logChoose(N,K);
 } 
function HyperGeomTail(k,n,K,N, sum,term,i,logTerm) { 
    # print "HyperGeomTail",k,n,K,N
    # 166 1113 2141 3436
    ASSERT(k<=K && k<=n && K<=N && n<=N && n-k<=N-K && K-k<=N-n,"HyperGeomTail: impossible (k,n,K,N)=("k","n","K","N")")
    if(k==0 && K>0) return 1; 
    if(tree_elem_exists("_hyperGeomMem", k) && tree_elem_exists("_hyperGeomMem", k, n)) 
 return query_tree("_hyperGeomMem", k, n, K, N)
    logTerm = logHyperGeomPMF(k,n,K,N)
    sum = term = Exp(logTerm)
    for(i=k+1; (i<=n && i<=K && sum < 0.999999) && (logTerm<723 || (sum && (term/sum > 1e-20))); i++) 
 # print i,logTerm,term,sum,sum-1
 logTerm = logHyperGeomPMF(i,n,K,N)
 term = Exp(logTerm)
 sum += term
     } 
    # print "DONE!!!",sum
    if(sum==0) sum=1e-320
    tree_insert("_hyperGeomMem", k, n, K, N, sum)
    return sum
 } 
function logHyperGeomTail(k,n,K,N, logSum,logTerm,i) { 
    # print "logHyperGeomTail",k,n,K,N
    ASSERT(k<=K && k<=n && K<=N && n<=N && n-k<=N-K && K-k<=N-n,"HyperGeomTail: impossible (k,n,K,N)=("k","n","K","N")")
    if(k==0 && K>0) return 0; 
    if(tree_elem_exists("_logHyperGeomMem", k) && tree_elem_exists("_logHyperGeomMem", k, n)) 
 return query_tree("_logHyperGeomMem", k, n, K, N)
    logTerm = logHyperGeomPMF(k,n,K,N)
    logSum = logTerm
    for(i=k+1; i<=n && i<=K && logTerm-logSum > -40; i++) 
 logTerm = logHyperGeomPMF(i,n,K,N)
 logSum  = LogSumLogs(logSum,logTerm)
     } 
    tree_insert("_logHyperGeomMem", k, n, K, N, logSum)
    return logSum
 } 
function StatRV_Normal() { if(!_StatRV_which) { do { _StatRV_v1 = 2*rand()-1; _StatRV_v2 = 2*rand()-1; _StatRV_rsq = _StatRV_v1^2+_StatRV_v2^2;  } while(_StatRV_rsq >= 1 || _StatRV_rsq == 0); _StatRV_fac=sqrt(-2*log(_StatRV_rsq)/_StatRV_rsq); _StatRV_next = _StatRV_v1*_StatRV_fac; _StatRV_which = 1; return _StatRV_v2*_StatRV_fac;  } else { _StatRV_which = 0; return _StatRV_next;  }  } 
function NormalRV(mu,sigma) { return mu+StdNormRV()*sigma } 
# The Spearman correlation is just the Pearson correlation of the rank. It measures monotonicity, not linearity.
# Unfortunately it means we need to store every sample, and sort them into rank order when we want the coefficient.
function SpearmanReset(name) { 
    tree_remove("_SpComputeResult", name); 
    tree_remove("_Spearman_N", name); 
    tree_remove("_SpearmanSampleX", name); 
    tree_remove("_SpearmanSampleY", name); 
    tree_remove("_Spearman_rho", name); 
    tree_remove("_Spearman_p", name); 
    tree_remove("_Spearman_t", name); 
 } 
function SpearmanAddSample(name,X,Y) { 
    tree_remove("_SpComputeResult", name); 
    _SpN = (query_tree("_Spearman_N", name)++)# 1-indexed, not zero.
    tree_insert("_SpearmanSampleX", name, _SpN, X); 
    tree_insert("_SpearmanSampleY", name, _SpN, Y); 
 } 
function SpearmanCompute(name, i,a,n) { 
    ASSERT(tree_elem_exists("_Spearman_N", name), "SpearmanCompute: no such data "name); 
    if(tree_elem_exists("_SpComputeResult", name)) return query_tree("_SpComputeResult", name); 
    tree_insert("ASSERT(length(_SpearmanSampleX", name, =length(query_tree("_SpearmanSampleY", name)), "SpearmanCompute: input arrays are different lengths")); 
    # Too hard to do this in awk, just run external spearman program
    _SpCommand = "spearman"
    for(i=1; i<=_Spearman_N[name]; i++) print query_tree("_SpearmanSampleX", name, i),query_tree("_SpearmanSampleY", name, i) |& _SpCommand; 
    close(_SpCommand,"to"); 
    _SpCommand |& getline query_tree("_SpComputeResult", name)
    close(_SpCommand,"from"); 
    n = split(query_tree("_SpComputeResult", name),a); 
    # ASSERT(a[1]==_Spearman_N[name],"SpearmanCompute: first field returned by external command "_SpCommand" is not _Spearman_N["name"]="_Spearman_N[name]);
    if(query_tree("a", 1)!=query_tree("_Spearman_N", name)) { 
 Warn("SpearmanCompute: external spearman returned " query_tree("_SpComputeResult", name)); 
 Warn("SpearmanCompute: but first field is not _Spearman_N["name"]="_Spearman_N[name]); 
     } 
    tree_insert("_Spearman_rho", name, query_tree("a", 2)); 
    tree_insert("_Spearman_p", name, query_tree("a", 3)); 
    tree_insert("_Spearman_t", name, query_tree("a", 4)); 
    delete_tree("a")
    return query_tree("_SpComputeResult", name); 
 } 
function SpearmanPrint(name) { return SpearmanCompute(name) } 
function CovarReset(name) { 
    tree_remove("_Covar_sumX", name)
    tree_remove("_Covar_sumY", name)
    tree_remove("_Covar_sumXY", name)
    tree_remove("_Covar_N", name)
 } 
function CovarAddSample(name,X,Y) { 
    tree_increment("_Covar_sumX", name, X)
    tree_increment("_Covar_sumY", name, Y)
    tree_increment("_Covar_sumXY", name, X*Y)
    tree_increment("_Covar_N", name, 1); 
 } 
function CovarCompute(name) { 
    ASSERT(1*query_tree("_Covar_N", name)>1, "CovarCompute requires N>=1 but it is "query_tree("_Covar_N", name)); 
    return (query_tree("_Covar_sumXY", name)-query_tree("_Covar_sumX", name)*query_tree("_Covar_sumY", name)/query_tree("_Covar_N", name))/(query_tree("_Covar_N", name)-1); 
 } 
function PearsonReset(name) { 
    tree_remove("_Pearson_sumX", name)
    tree_remove("_Pearson_sumY", name)
    tree_remove("_Pearson_sumXY", name)
    tree_remove("_Pearson_sumX2", name)
    tree_remove("_Pearson_sumY2", name)
    tree_remove("_Pearson_N", name)
    tree_remove("_Pearson_rho", name)
    tree_remove("_Pearson_t", name)
    tree_remove("_Pearson_p", name)
 } 
function PearsonAddSample(name,X,Y) { 
    tree_insert("_PearsonComputeValid", name, 0); 
    tree_increment("_Pearson_sumX", name, X)
    tree_increment("_Pearson_sumY", name, Y)
    tree_increment("_Pearson_sumXY", name, X*Y)
    tree_increment("_Pearson_sumX2", name, X*X)
    tree_increment("_Pearson_sumY2", name, Y*Y)
    tree_increment("_Pearson_N", name, 1); 
 } 
function Pearson2T(n,r) { if(r==1) return BIGNUM; else return r*sqrt((n-2)/(1-r^2)) } 
function PearsonCompute(name,     numer,DX,DY,denom,z,zse,F) { 
    if(!query_tree("_Pearson_N", name)) return 0; 
    if(query_tree("_PearsonComputeValid", name)) return 1; 
    numer = query_tree("_Pearson_sumXY", name)-query_tree("_Pearson_sumX", name)*query_tree("_Pearson_sumY", name)/query_tree("_Pearson_N", name)
    DX = query_tree("_Pearson_sumX2", name)-query_tree("_Pearson_sumX", name)*query_tree("_Pearson_sumX", name)/query_tree("_Pearson_N", name)
    DY = query_tree("_Pearson_sumY2", name)-query_tree("_Pearson_sumY", name)*query_tree("_Pearson_sumY", name)/query_tree("_Pearson_N", name)
    # print DX,DY >"/dev/stderr"
    denom=sqrt(ABS(DX*DY)); # ABS since sometimes it is very slightly negative due to rounding errors
    if(denom==0) { Warn("PearsonCompute("name"): denom is zero"); return 0 } ; 
    tree_insert("_Pearson_rho", name, numer/denom); 
    # Fisher R-to-z
    if(query_tree("_Pearson_rho", name)==1) { 
 tree_insert("_Pearson_t", name, PI*query_tree("_Pearson_N", name)); # pulled these out of a hat...
 tree_insert("_Pearson_p", name, exp(-query_tree("_Pearson_t", name))); 
     } else { 
 tree_insert("_Pearson_t", name, Pearson2T(query_tree("_Pearson_N", name),query_tree("_Pearson_rho", name))); 
 if(query_tree("_Pearson_t", name)<0) tree_insert("_Pearson_t", name, -query_tree("_Pearson_t", name)); 
 z = 0.5*log((1+query_tree("_Pearson_rho", name))/(1-query_tree("_Pearson_rho", name)))
 zse = 1/sqrt(ABS(query_tree("_Pearson_N", name)-3))
 tree_insert("_Pearson_p", name, F=2*MIN(NormalDist(0,zse,z),NormalDist(0,zse,-z)))
     } 
    # We seem to be at least 100x too small according to Fisher
    if(1*query_tree("_Pearson_p", name)>1) tree_insert("_Pearson_p", name, 1-1/query_tree("_Pearson_p", name))
    tree_insert("_PearsonComputeValid", name, 1); 
    return 1
 } 
function PearsonPrint(name, logp) { 
    # if(!_Pearson_N[name]) return;
    if(PearsonCompute(name)) { 
 # if(_Pearson_rho[name]==1) return sprintf("%d %.4g %.4g %.4f", _Pearson_N[name], _Pearson_rho[name], 0, _Pearson_t[name])
 TINY=1e-200; # using the fancy log algorithm if p-value is smaller than this
 logp = -logPhi(-query_tree("_Pearson_t", name)); # working with the negative log is easier (so log is positive)
 if(logp < -log(TINY)) 
     return sprintf("%d\t%.4g\t%.4g\t%.4f", query_tree("_Pearson_N", name), query_tree("_Pearson_rho", name), query_tree("_Pearson_p", name), query_tree("_Pearson_t", name))
 else { 
     # printf "t %g p %g log10p %g logp %g", _Pearson_t[name], _Pearson_p[name], logp/log(10), logp > "/dev/stderr"
     logp = (logp - 8.28931 - logp/65.1442)/0.992# Empirical correction to get in line with Fisher for small p-values
     # printf " (logp corrected %g %g)\n", logp/log(10), logp > "/dev/stderr"
     return sprintf("%d\t%.4g\t%s\t%.4f (using log)",query_tree("_Pearson_N", name),query_tree("_Pearson_rho", name),logPrint(-logp,4),query_tree("_Pearson_t", name)); 
     # p=10^-logp; print "log-over-Fisher", p/F # Sanity check
  } 
     } 
    else return ""; 
 } 
# Functions for computing the AUPR
function AUPR_add(name, value, thresh, truth) { 
    tree_increment("_AUPR_N", name, 1); 
    if(value > thresh) { # predicted
 if(truth) tree_increment("_AUPR_TP", name, 1); else tree_increment("_AUPR_FP", name, 1)
     } else { # not predicted
 if(truth) tree_increment("_AUPR_FN", name, 1); else tree_increment("_AUPR_TN", name, 1)
     } 
 } 
function AUPR_Prec(name,  TP, FP) { TP = query_tree("_AUPR_TP", name); FP = query_tree("_AUPR_FP", name); if(TP+FP==0) return 1; else return TP/(TP+FP) } 
function AUPR_Rec(name,     TP, FN) { TP = query_tree("_AUPR_TP", name); FN = query_tree("_AUPR_FN", name); if(TP+FN==0) return 1; else return TP/(TP+FN) } 
function AUPR_F1(name,         Prec,Rec) { Prec=AUPR_Prec(name); Rec=AUPR_Rec(name); 
    if(Prec+Rec==0) return 0; else return 2*Prec*Rec/(Prec+Rec)
 } 
function AUPR_TPR(name,  TP, FN) { TP = query_tree("_AUPR_TP", name); FN = query_tree("_AUPR_FN", name); return TP/(TP+FN) } 
function AUPR_FPR(name,  FP, TN) { FP = query_tree("_AUPR_FP", name); TN = query_tree("_AUPR_TN", name); return FP/(FP+TN) } 
# The method of least squares is a standard technique used to find
# the equation of a straight line from a set of data. Equation for a
# straight line is given by
# y = mx + b
# where m is the slope of the line and b is the y-intercept.
# 
# Given a set of n points {(x1,y1), x2,y2),...,xn,yn)}, let
# SUMx = x1 + x2 + ... + xn
# SUMy = y1 + y2 + ... + yn
# SUMxy = x1*y1 + x2*y2 + ... + xn*yn
# SUMxx = x1*x1 + x2*x2 + ... + xn*xn
# 
# The slope and y-intercept for the least-squares line can be
# calculated using the following equations:
# slope (m) = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx )
# y-intercept (b) = ( SUMy - slope*SUMx ) / n
# AUTHOR: Dora Abdullah (Fortran version, 11/96)
# REVISED: RYL (converted to C, 12/11/96)
# Converted to awk: Wayne Hayes (2020-March-09)
function LS_Reset(name) { tree_insert("_LS_valid", name, query_tree("_LS_SUMx", name)=query_tree("_LS_SUMy", name)=query_tree("_LS_SUMxy", name)=query_tree("_LS_SUMxx", name)=query_tree("_LS_n", name)=0); tree_remove("_LS_x", name); tree_remove("_LS_y", name) } 
function LS_Sample(name,x,y) { tree_insert("_LS_valid", name, 0); tree_increment("_LS_SUMx", name, x); tree_increment("_LS_SUMy", name, y); tree_increment("_LS_SUMxy", name, x*y); tree_increment("_LS_SUMxx", name, x*x); tree_insert("_LS_x", name, query_tree("_LS_n", name), x); tree_insert("_LS_y", name, query_tree("_LS_n", name), y); tree_increment("_LS_n", name, 1) } 
function LS_Slope(name, denom) { denom = ( query_tree("_LS_SUMx", name)*query_tree("_LS_SUMx", name) - query_tree("_LS_n", name)*query_tree("_LS_SUMxx", name)); if(!denom) return 0; 
    return tree_insert("_LS_slope", name, (query_tree("_LS_SUMx", name)*query_tree("_LS_SUMy", name) - query_tree("_LS_n", name)*query_tree("_LS_SUMxy", name))/denom) } 
function LS_Yintercept(name) { return ( query_tree("_LS_SUMy", name) - LS_Slope(name)*query_tree("_LS_SUMx", name) ) / query_tree("_LS_n", name) } 
function LS_Xintercept(name) { if(LS_Slope(name)) return -LS_Yintercept(name) / LS_Slope(name); else return BIGNUM;  } 
function LS_Predict(name, x) { 
    if(query_tree("_LS_n", name)>0 && (query_tree("_LS_SUMx", name)*query_tree("_LS_SUMx", name) - query_tree("_LS_n", name)*query_tree("_LS_SUMxx", name)) != 0) { 
 return LS_Slope(name)*x + LS_Yintercept(name); 
     } 
 } 
function LS_Compute(name,   slope,y_intercept,y_estimate,res,i) { 
    if(query_tree("_LS_n", name) && !query_tree("_LS_valid", name)) { 
 tree_insert("_LS_SUMres", name, 0); 
 tree_insert("_LS_SUMres2", name, 0); 
 slope = LS_Slope(name); 
 y_intercept = LS_Yintercept(name); 
 for (i=0; i<_LS_n[name]; i++) 
     y_estimate = slope*query_tree("_LS_x", name, i) + y_intercept; 
     res = query_tree("_LS_y", name, i) - y_estimate; 
     tree_increment("_LS_SUMres", name, res); 
     tree_increment("_LS_SUMres2", name, res*res); 
  } 
     } 
    return (tree_insert("_LS_valid", name, query_tree("_LS_n", name))); # automatically make it invalid if n=0
 } 
function LS_R2(name,  SUMres,SUMres2,slope,y_intercept,y_estimate,res,i) { 
    LS_Compute(name); 
    return query_tree("_LS_SUMres2", name); 
 } 
function LS_Variance(name,  SUMres,SUMres2,slope,y_intercept,y_estimate,res,i) { 
    if(1*query_tree("_LS_n", name)<2) return 0; 
    LS_Compute(name); 
    return (query_tree("_LS_SUMres2", name) - query_tree("_LS_SUMres", name)*query_tree("_LS_SUMres", name)/query_tree("_LS_n", name))/(query_tree("_LS_n", name)-1); 
 } 
function LS_MSR(name) { 
  return LS_R2(name)/query_tree("_LS_n", name); 
 } 
# ################ GRAPH ROUTINES ##################
# return true if (u,v) is NOT an edge
function IsEdge(edge,u,v) { return (tree_elem_exists("edge", u))&&(tree_elem_exists("edge", u, v);  } 
# Input: edgeList; a single node, u, to start the BFS; and an (optional) "searchNode" to stop at.
# Output: array dist[] contains shortest paths from u to all nodes reachable from u within maxDist; includes dist[u]=0.
# Call with maxDist=n (size of network) to get the BFS distance to everybody
function BFS(edge,u,searchNode,dist,   V,Q,m,M,x,y) { 
    ASSERT(isarray(edge), "BFS: edgeList must be binary symmetric 2D array"); 
    delete_tree("V"); # visited
    delete_tree("Q"); # queue
    delete_tree("dist"); # distance from u
    tree_insert("dist", u, 0); 
    m=M=0; 
    tree_insert("Q", M++, u); # the BFS queue runs from m [inclusive] to M-1, and we increment m as we dequeue elements
    while(M>m) { 
 x = query_tree("Q", m++); 
 ASSERT(tree_elem_exists("dist", x), x" in Q but tree_elem_exists("distance", not) array"); 
 if(!(x in V)) { 
     tree_insert("V", x, 1); 
     ASSERT(isarray(query_tree("edge", x)), "query_tree("edge", "x") is not an array"); 
     while (tree_iters_remaining("edge", x) > 0) { y = tree_next("edge", x); if(!(y in V)) { 
 }   if(tree_elem_exists("dist", y)) tree_insert("dist", y, MIN(query_tree("dist", y),query_tree("dist", x)+1)); 
  else tree_insert("dist", y, query_tree("dist", x)+1); 
  if(y==searchNode) return; 
  tree_insert("Q", M++, y); 
      } 
  } 
     } 
 } 
function MakeEmptySet(S) { delete_tree("S"); tree_insert("S", 0, 1); tree_remove("S", 0) } 
function InducedEdges(edge,T,D,       u,v,m) { # note you can skip passing in D
    MakeEmptySet(D); 
    while (tree_iters_remaining("T") > 0) { u = tree_next("T"); while (tree_iters_remaining("T") > 0) { v = tree_next("T"); if((tree_elem_exists("edge", u)) && (tree_elem_exists("edge", u, v))) { tree_increment("D", u, 1); tree_increment("D", v, 1); ++m;  }  }  } 
    while (tree_iters_remaining("T") > 0) { u = tree_next("T"); tree_insert("ASSERT(D", u, =0, "InducedEdges: D["u"]="D[u])); tree_modify("D", u, "/2");  } 
    ASSERT(m%2==0, "m is not even"); 
    return m/2; 
 } 
function InducedWeightedEdges(edge,T,D,       u,v,m,all1) { # note you can skip passing in D
    MakeEmptySet(D); all1=1; 
    while (tree_iters_remaining("T") > 0) { u = tree_next("T"); while (tree_iters_remaining("T") > 0) { v = tree_next("T"); if((tree_elem_exists("edge", u)) && (tree_elem_exists("edge", u, v))) { 
 }  }  if(query_tree("edge", u, v) != 1) all1 = 0; 
 tree_increment("D", u, query_tree("edge", u, v)); tree_increment("D", v, query_tree("edge", u, v)); m += query_tree("edge", u, v); 
     } 
    while (tree_iters_remaining("T") > 0) { u = tree_next("T"); if(all1) tree_insert("ASSERT(D", u, =0, "InducedEdges: D["u"]="D[u])); tree_modify("D", u, "/2");  } 
    if(all1) ASSERT(m%2==0, "m is not even"); 
    return m/2; 
 } 
# Input: the edge list, the degree array, and the node on which to compute local density
function LocalEdgeDensity(edge,D,u, v,w,n,set) { # crude estimate of local edge density around a node
    tree_increment("set", u, 1); 
    while (tree_iters_remaining("edge", u) > 0) { v = tree_next("edge", u); 
 tree_increment("set", v, 1); n += query_tree("D", v)-1; 
 while (tree_iters_remaining("edge", v) > 0) { w = tree_next("edge", v); tree_increment("set", w, 1);  } 
     } 
    return n/2/choose(length(set),2)
 } 
# Note: Possible sort orders are: "@unsorted",
# "@ind_str_asc",	"@ind_num_asc",	 "@val_type_asc",  "@val_str_asc",  "@val_num_asc",
# "@ind_str_desc",	"@ind_num_desc", "@val_type_desc", "@val_str_desc", "@val_num_desc",
# This implementation allows multiple elements with the same priority... and even multiple [p][element] duplicates
function PQpush(name, pri, element) { tree_increment("_PQ_", name, pri, element, 1); tree_increment("_PQ_size", name, 1) } 
function PQpop(name,    prevSort,element,p) { 
    prevSort = PROCINFO["sorted_in"]; # remember sort order to restore it afterwards
    tree_insert("PROCINFO", "sorted_in", "@ind_num_desc"); 
    while (tree_iters_remaining("_PQ_", name) > 0) { p = tree_next("_PQ_", name); 
        # Note that if multiple elements have the same priority, we will return them in SORTED order not INSERTION order
        while (tree_iters_remaining("_PQ_", name, p) > 0) { element = tree_next("_PQ_", name, p); 
            if(--query_tree("_PQ_", name, p, element)==0) { 
                tree_remove("_PQ_", name, p, element); 
                if(tree_length("_PQ_[name][p]")==0) tree_remove("_PQ_", name, p); 
             } 
            tree_iter_break("_PQ_", name); break; # exit at first iteration
         } 
        break; # exit at first iteration
     } 
    tree_insert("PROCINFO", "sorted_in", prevSort); # restore sort order
    tree_decrement("_PQ_size", name, 1)
    return element; 
 } 
function PQlength(name) { return query_tree("_PQ_size", name);  } 
function PQalloc(name) { tree_insert("_PQ_size", name, 0); tree_insert("PQ_", name, 0, 0, 1); tree_remove("PQ_", name, 0) } 
function PQdelloc(name) { tree_remove("PQ_size", name); tree_remove("PQ_", name) } 
function PQfree(name) { PQdelloc(name);  } 
 { ASSERT(!gsub("
",""), "Sorry, we cannot accept DOS text files. Please remove the carriage returns from file "FILENAME);  } 

