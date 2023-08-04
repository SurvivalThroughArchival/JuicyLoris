#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>


#include "AiffData.h"
#include "AiffFile.h"
#include "Marker.h"

#include "Analyzer.h"
#include "BreakpointEnvelope.h"
#include "Channelizer.h"
#include "Dilator.h"
#include "Distiller.h"
#include "Exception.h"
#include "FrequencyReference.h"
#include "Partial.h"
#include "PartialList.h"
#include "PartialUtils.h"
#include "SdifFile.h"
#include "LorisExceptions.h"
#include "Morpher.h"
#include "Notifier.h"
#include "Resampler.h"
#include <iostream>
#include <list>
#include <cstdlib>
#include <string>
#include <stdexcept>

#define ESTIMATE_F0 0
#define ESTIMATE_AMP 0
//
//#include "loris/loris.h"  // just for version string

using namespace Loris;

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::atof;

    void
    importSamples( const string & path,
                  vector< double > & buffer,
                  double * srate = 0 )
    {
        AiffFile fin( path );
        
        buffer = fin.samples();
        
        if ( 0 != srate )
        {
            *srate = fin.sampleRate();
        }
    }
    
    // ---------------------------------------------
    //  dumpEnvelope
    // ---------------------------------------------
    //  Dump envelope statistics, return the mean.
    
    double dumpEnvelope( const LinearEnvelope & freq )
    {
        cout << "dumping envelope" << endl;
        
        double N = freq.size(); // changed from unsigned int
        double sum = 0;
        
        for ( LinearEnvelope::const_iterator it = freq.begin();
             it != freq.end();
             ++it )
        {
            double t = it->first;
            double f = it->second;
            
            cout << "time: " << t << "\t value: " << f << "\n";
            
            sum += f;
        }
        
        double mean = sum / (double)N;
        cout << "mean: " << mean << endl;
        
        return mean;
    }

int main (int argc, char** argv)
{
    bool printActions = true;
    cout << "Loris Spectral Library Testing" << endl;

    // Get starting timepoint
    auto start = std::chrono::high_resolution_clock::now();

    //        analyze clarinet tone
    if (printActions)
        cout << "importing clarinet samples" << endl;

    Analyzer a( 415.0*.8, 415.0*1.6 );
    //            a.analyzer_storeNoBandwidth();
    AiffFile f( "./clarinet.aiff" );
    
    // analyze the clarinet
    if (printActions)
        cout << "analyzing clarinet 4G#" << endl;
    a.analyze( f.samples(), f.sampleRate()  );
    PartialList clar = a.partials();
    
    // channelize and distill
    if (printActions)
        cout << "distilling" << endl;
    FrequencyReference clarRef( clar.begin(), clar.end(), 415*.8, 415*1.2, 50 );
    Channelizer::channelize( clar, clarRef , 1 );
    Distiller::distill( clar, 0.001 );
    
    //    test SDIF import and export
    if (printActions)
        cout << "exporting " << clar.size() << " partials to SDIF file" << endl;
    SdifFile::Export( "./results/clarinet.sdif", clar );
    if (printActions)
        cout << "importing from SDIF file" << endl;
    SdifFile ip("./results/clarinet.sdif");
    if ( clar.size() != ip.partials().size() )
    {
        throw std::runtime_error( "SDIF import yields a different number of partials than were exported!" );
    }
    clar = ip.partials();
    
    // shift pitch of clarinet partials
    if (printActions)
        cout << "shifting pitch of " << clar.size() << " Partials by 600 cents" << endl;
    PartialUtils::shiftPitch( clar.begin(), clar.end(), -600 );
    
    // check clarinet synthesis
    if (printActions)
        cout << "checking clarinet synthesis" << endl;
    AiffFile clarout( clar.begin(), clar.end(), f.sampleRate() );
    clarout.write( "./results/clarOK1.aiff" );
    
    //    analyze flute tone
    if (printActions)
        cout << "importing flute samples" << endl;
    f = AiffFile( "./flute.aiff" );
    
    // analyze the flute
    if (printActions)
        cout << "analyzing flute 4D" << endl;
    a = Analyzer( 270 );
#if defined(ESTIMATE_F0) && ESTIMATE_F0
    cout << "Analyzer will build a fundamental frequency estimate for the flute" << endl;
    a.buildFundamentalEnv( 270, 310 );
#endif
#if defined(ESTIMATE_AMP) && ESTIMATE_AMP
    a.buildAmpEnv( true );
#endif
    a.analyze( f.samples(), f.sampleRate() );
    PartialList flut = a.partials();
    
    // channelize and distill
    if (printActions)
        cout << "distilling" << endl;
#if defined(ESTIMATE_F0) && ESTIMATE_F0
    const LinearEnvelope & flutRef = a.fundamentalEnv();
    double est_time = flutRef.begin()->first;
    cout << "flute fundamental envelope starts at time " << est_time << endl;
    while ( est_time < 2 )
    {
        cout << "flute fundamental estimate at time "
        << est_time << " is " << flutRef.valueAt( est_time ) << endl;
        est_time += 0.35;
    }
#else
    FrequencyReference flutRef( flut.begin(), flut.end(), 291*.8, 291*1.2, 50 );
#endif
    Channelizer::channelize( flut, flutRef, 1 );
    Distiller::distill( flut, 0.001 );
    if (printActions)
        cout << "obtained " << flut.size() << " distilled flute Partials" << endl;
    
#if defined(ESTIMATE_F0) && ESTIMATE_F0
#if defined(ESTIMATE_AMP) && ESTIMATE_AMP
    //  generate a sinusoid that tracks the fundamental
    //    and amplitude envelopes obtained during analysis
    if (printActions)
        cout << "synthesizing sinusoid from flute amp and fundamental estimates" << endl;
    Partial boo;
    LinearEnvelope fund = a.fundamentalEnv();
    LinearEnvelope::iterator it;
    for ( it = fund.begin(); it != fund.end(); ++it )
    {
        Breakpoint bp( it->second, a.ampEnv().valueAt( it->first ), 0, 0 );
        boo.insert( it->first, bp );
    }
    
    PartialList boolist;
    boolist.push_back( boo );
    AiffFile boofile( boolist.begin(), boolist.end(), 44100 );
    boofile.write( "./results/flutefundamental.aiff" );
    
#endif
#endif
    if (printActions)
        cout << "exporting " << flut.size() << " partials to SDIF file" << endl;
    SdifFile::Export( "./results/flute.sdif", flut );
    
    // check flute synthesis:
    if (printActions)
        cout << "checking flute synthesis" << endl;
    AiffFile flutout( flut.begin(), flut.end(), f.sampleRate() );
    flutout.write( "./results/flutOK.aiff" );
    
    // perform temporal dilation
    double flute_times[] = { 0.4, 1. };
    double clar_times[] = { 0.2, 1. };
    double tgt_times[] = { 0.0, 1.2 };
    if (printActions)
    {
        cout << "dilating sounds to match (" << tgt_times[0] << ", " << tgt_times[1] << ")" << endl;
        cout << "flute times: (" << flute_times[0] << ", " << flute_times[1] << ")" << endl;
    }

    Dilator::dilate(  flut.begin(), flut.end() , flute_times, flute_times+2, tgt_times );
    
    if (printActions)
    {
        cout << "clarinet times: (" << clar_times[0] << ", " << clar_times[1] << ")" << endl;
    }

    Dilator::dilate(  clar.begin(), clar.end(), clar_times, clar_times+2, tgt_times );
    
    // perform morph
    if (printActions)
        cout << "morphing flute and clarinet" << endl;
    BreakpointEnvelope mf;
    mf.insertBreakpoint( 0.6, 0 );
    mf.insertBreakpoint( 2, 1 );
    
    Morpher m( mf );
    m.setMinBreakpointGap( 0.002 );
    m.setSourceReferencePartial( clar, 3 );
    m.setTargetReferencePartial( flut, 1 );
    m.morph( clar.begin(), clar.end(), flut.begin(), flut.end() );
    
    
    // synthesize and export samples
    if (printActions)
        cout << "synthesizing " << m.partials().size() << " morphed partials" << endl;
    AiffFile morphout( m.partials().begin(), m.partials().end(), f.sampleRate() );
    morphout.write( "./results/morph.aiff" );

       // Get ending timepoint
    auto stop = std::chrono::high_resolution_clock::now();
 
    // Get duration. Substart timepoints to
    // get duration. To cast it to proper unit
    // use duration cast method
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
 
    cout << "Time taken by function: "
         << duration.count() * 0.000001 << " seconds" << endl;

    cout << "Done, bye." << endl;

    return EXIT_SUCCESS;
}
