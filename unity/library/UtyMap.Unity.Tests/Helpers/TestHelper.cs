using System;
using UtyMap.Unity;
using UtyMap.Unity.Core;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyDepend;
using UtyDepend.Config;

namespace UtyMap.Unity.Tests.Helpers
{
    internal static class TestHelper
    {
        #region Constants values

        public const string IntegrationTestCategory = "Integration";

        public static GeoCoordinate WorldZeroPoint = new GeoCoordinate(52.5317429, 13.3871987);

        public const string TestAssetsFolder = @"../../../../demo/Assets/Resources";
        public const string ConfigTestRootFile = TestAssetsFolder + @"/Config/test.json";
        public const string BerlinXmlData = TestAssetsFolder + @"/Osm/berlin.osm.xml";
        public const string BerlinPbfData = TestAssetsFolder + @"/Osm/berlin.osm.pbf";
        public const string NmeaFilePath = TestAssetsFolder + @"/Nmea/invalidenstrasse_borsigstrasse.nme";
        public const string DefaultMapCss = TestAssetsFolder + @"/MapCss/default/default.mapcss";

        #endregion

        public static CompositionRoot GetCompositionRoot(GeoCoordinate worldZeroPoint)
        {
            return GetCompositionRoot(worldZeroPoint, (container, section) => { });
        }

        public static CompositionRoot GetCompositionRoot(GeoCoordinate worldZeroPoint,
            Action<IContainer, IConfigSection> action)
        {
            // create default container which should not be exposed outside
            // to avoid Service Locator pattern.
            IContainer container = new Container();

            // create default application configuration
            var config = ConfigBuilder.GetDefault()
                .SetStringIndex("Index/")
                .SetSpatialIndex("Index/")
                .Build();

            // initialize services
            return new CompositionRoot(container, config)
                .RegisterAction((c, _) => c.Register(Component.For<ITrace>().Use<ConsoleTrace>()))
                .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<TestPathResolver>()))
                .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(DefaultMapCss)))
                .RegisterAction((c, _) => c.Register(Component.For<IProjection>().Use<CartesianProjection>(worldZeroPoint)))
                .RegisterAction(action)
                .Setup();
        }
    }
}
