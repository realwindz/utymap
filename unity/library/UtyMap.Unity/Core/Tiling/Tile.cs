using System;
using UnityEngine;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Primitives;

namespace UtyMap.Unity.Core.Tiling
{
    /// <summary> Represents map tile. </summary>
    public class Tile
    {
        /// <summary> Tile rectamgle in world coordinates. </summary>
        public Rectangle Rectangle { get; private set; }

        /// <summary> Tile geo bounding box. </summary>
        public BoundingBox BoundingBox { get; private set; }

        /// <summary> Corresponding quadkey. </summary>
        public QuadKey QuadKey { get; private set; }

        /// <summary> Stylesheet. </summary>
        public Stylesheet Stylesheet { get; private set; }

        /// <summary> Used projection. </summary>
        public IProjection Projection { get; private set; }

        /// <summary> Sets game object which holds all children objects. </summary>
        public GameObject GameObject { get; set; }

        /// <summary> Stores element ids loaded in this tile. </summary>
        private readonly SafeHashSet<long> _localIds = new SafeHashSet<long>();

        /// <summary> Stores element ids loaded for all tiles. </summary>
        private static readonly SafeHashSet<long> GlobalIds = new SafeHashSet<long>();

        /// <summary> Creates <see cref="Tile"/>. </summary>
        /// <param name="quadKey"></param>
        /// <param name="stylesheet"></param>
        /// <param name="projection"> Projection. </param>
        internal Tile(QuadKey quadKey, Stylesheet stylesheet, IProjection projection)
        {
            QuadKey = quadKey;
            Stylesheet = stylesheet;
            Projection = projection;

            BoundingBox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            Rectangle = GeoUtils.QuadKeyToRect(projection, quadKey);
        }

        /// <summary> Checks whether absolute position locates in tile with bound offset. </summary>
        /// <param name="position">Absolute position in game.</param>
        /// <param name="offset">offset from bounds.</param>
        /// <returns>Tres if position in tile</returns>
        public bool Contains(Vector2 position, double offset)
        {
            return (position.x > Rectangle.Left + offset) && (position.y < Rectangle.Top - offset) &&
                   (position.x < Rectangle.Right - offset) && (position.y > Rectangle.Bottom + offset);
        }

        /// <inheritdoc />
        public override string ToString()
        {
            return String.Format("({0},{1}:{2})", QuadKey.TileX, QuadKey.TileY, QuadKey);
        }

        /// <summary> Checks whether element with specific id is registered. </summary>
        /// <param name="id">Element id.</param>
        /// <returns>True if registration is found.</returns>
        public bool Has(long id)
        {
            return GlobalIds.Contains(id);
        }

        /// <summary> Register element with given id inside to prevent multiple loading. </summary>
        public void Register(long id)
        {
            _localIds.Add(id);
            GlobalIds.Add(id);
        }

        #region IDisposable implementation

        /// <inheritdoc />
        public void Dispose()
        {
            // remove all registered ids from global list if they are in current registry
            foreach (var id in _localIds)
                GlobalIds.Remove(id);
            _localIds.Clear();
        }

        #endregion
    }
}
