using System;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyDepend;

namespace UtyMap.Unity.Maps.Data
{
    /// <summary> Specifies behavior of element editor. </summary>
    public interface IElementEditor
    {
        /// <summary> Adds element. </summary>
        void Add(Element element, Range<int> levelOfDetails);

        /// <summary> Edits element. </summary>
        void Edit(Element element, Range<int> levelOfDetails);

        /// <summary> Marks element with given id. </summary>
        void Delete(long elementId, Range<int> levelOfDetails);
    }

    /// <summary>
    ///     Default implementation of <see cref="IElementEditor"/> which
    ///     works with in-memory store.
    /// </summary>
    internal class ElementEditor : IElementEditor
    {
        private readonly Stylesheet _stylesheet;
        private readonly IPathResolver _resolver;

        [Dependency]
        public ElementEditor(Stylesheet stylesheet, IPathResolver resolver)
        {
            _stylesheet = stylesheet;
            _resolver = resolver;
        }

        /// <inheritdoc />
        public void Add(Element element, Range<int> levelOfDetails)
        {
            CoreLibrary.AddElementToInMemoryStore(_resolver.Resolve(_stylesheet.Path),
                element, levelOfDetails, message =>
                {
                    if (!String.IsNullOrEmpty(message))
                        throw new MapDataException(message);
                });
        }

        /// <inheritdoc />
        public void Edit(Element element, Range<int> levelOfDetails)
        {
            throw new NotImplementedException();
        }

        /// <inheritdoc />
        public void Delete(long elementId, Range<int> levelOfDetails)
        {
            throw new NotImplementedException();
        }
    }
}
