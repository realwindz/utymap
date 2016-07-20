using System;
using System.IO;
using System.Threading;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Positioning;
using UtyMap.Unity.Core.Positioning.Nmea;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

namespace UtyMap.Unity.Tests
{
    /// <summary> This class is useful to debug/profile application independenly from Unity Editor. </summary>
    internal class Program
    {
        private const int LevelOfDetails = 16;
        private const string TraceCategory = "test.app";

        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private readonly ManualResetEvent _waitEvent = new ManualResetEvent(false);

        private CompositionRoot _compositionRoot;
        private IMessageBus _messageBus;
        private ITileController _tileController;

        private ITrace _trace;

        private static void Main(string[] args)
        {
            var program = new Program();
            program.Initialize();
            program.RunMocker();
        }

        private void Initialize()
        {
            // initialize services
            _compositionRoot = TestHelper.GetCompositionRoot(_worldZeroPoint);

            // get local references
            _messageBus = _compositionRoot.GetService<IMessageBus>();
            _trace = _compositionRoot.GetService<ITrace>();
            _tileController = _compositionRoot.GetService<ITileController>();

            SetupMapData();

            // set observer to react on geo position changes
            _messageBus.AsObservable<GeoPosition>()
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(position =>
            {
                _trace.Debug(TraceCategory, "GeoPosition: {0}", position.ToString());
                _tileController.OnPosition(position.Coordinate, LevelOfDetails);
            });
        }

        /// <summary> Reads nmea file and sends fake geo positions via message bus. </summary>
        private void RunMocker()
        {
            Action<TimeSpan> delayAction = Thread.Sleep;
            using (Stream stream = new FileStream(TestHelper.NmeaFilePath, FileMode.Open))
            {
                var mocker = new NmeaPositionMocker(stream, _messageBus);
                mocker.OnDone += (s, e) => _waitEvent.Set();
                mocker.Start(delayAction);
            }
        }

        /// <summary> Setup test map data. </summary>
        private void SetupMapData()
        {
            var mapDataLoader = _compositionRoot.GetService<IMapDataLoader>();
            var stylesheet = _compositionRoot.GetService<Stylesheet>();

            mapDataLoader.AddToInMemoryStore(TestHelper.BerlinXmlData, stylesheet,
                new Range<int>(LevelOfDetails, LevelOfDetails));
        }
    }
}