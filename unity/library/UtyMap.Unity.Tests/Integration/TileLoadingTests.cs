using System;
using UtyMap.Unity;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Infrastructure.Reactive;
using UtyMap.Unity.Maps.Data;
using NUnit.Framework;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

namespace UtyMap.Unity.Tests.Integration
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class TileLoadingTests
    {
        private const int LevelOfDetails = 16;

        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private CompositionRoot _compositionRoot;
        private IMapDataLoader _mapDataLoader;
        private ITileController _tileController;

        [TestFixtureSetUp]
        public void Setup()
        {
            // initialize services
            _compositionRoot = TestHelper.GetCompositionRoot(_worldZeroPoint);

            // get local references
            _mapDataLoader = _compositionRoot.GetService<IMapDataLoader>();
            _tileController = _compositionRoot.GetService<ITileController>();
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test(Description = "This test loads 64 tiles from osm file to ensure that there are no unexpected crashes at least at test region.")]
        public void CanLoadMultipleTiles()
        {
            // ARRANGE
            SetupMapData(TestHelper.BerlinPbfData);
            var count = 10;
            var centerQuadKey = new QuadKey(35205, 21489, LevelOfDetails);

            // ACT & ASSERT
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                {
                    var quadKey = new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, LevelOfDetails);
                    var tile = new Tile(quadKey, _tileController.Stylesheet, _tileController.Projection);
                    _mapDataLoader
                        .Load(tile)
                        .SubscribeOn(Scheduler.CurrentThread)
                        .ObserveOn(Scheduler.CurrentThread)
                        .Subscribe(AssertData);
                }
        }

        #region Private members

        /// <summary> Setup test map data. </summary>
        private void SetupMapData(string mapDataPath)
        {
            var range = new Range<int>(LevelOfDetails, LevelOfDetails);
            _compositionRoot
                .GetService<IMapDataLoader>()
                .AddToInMemoryStore(mapDataPath, _tileController.Stylesheet, range);
        }

        /// <summary> Checks whether data satisfied minimal correctness critirea. </summary>
        private void AssertData(Union<Element, Mesh> data)
        {
            var elements = 0;
            var meshes = 0;
            data.Match(
                element =>
                {
                    Assert.Greater(element.Geometry.Length, 0);
                    ++elements;
                }, mesh =>
                {
                    Assert.Greater(mesh.Vertices.Length, 0);
                    Assert.Greater(mesh.Triangles.Length, 0);
                    Assert.Greater(mesh.Colors.Length, 0);
                    ++meshes;
                });

            Assert.Greater(elements + meshes, 0);
        }

        #endregion
    }
}
