using Moq;
using NUnit.Framework;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace UtyMap.Unity.Tests.Maps.Data
{
    [TestFixture]
    public class MapTileAdapterTests
    {
        private Mock<IObserver<Union<Element, Mesh>>> _observer;
        private MapTileAdapter _adapter;
            
        [TestFixtureSetUp]
        public void SetUp()
        {
            var tile = new Tile(new QuadKey(), new Mock<Stylesheet>("").Object, new Mock<IProjection>().Object);
            _observer = new Mock<IObserver<Union<Element, Mesh>>>();
            _adapter = new MapTileAdapter(tile, _observer.Object, new DefaultTrace());
        }

        [TestCase("barrier")]
        [TestCase("building")]
        public void CanAdaptTheSameNonTerrainMeshOnlyOnce(string name)
        {
            name += ":42";

            _adapter.AdaptMesh(name, new[] {.0, 0, 0}, 3, new[] {0, 0, 0}, 3, new[] {0, 0, 0}, 3);
            _adapter.AdaptMesh(name, new[] { .0, 0, 0 }, 3, new[] { 0, 0, 0 }, 3, new[] { 0, 0, 0 }, 3);

            _observer.Verify(o => o.OnNext(It.IsAny<Union<Element, Mesh>>()), Times.Once);
        }
    }
}
