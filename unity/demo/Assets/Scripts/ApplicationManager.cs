using System;
using System.Collections.Generic;
using Assets.Scripts.Console;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Explorer.Customization;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Infrastructure.Reactive;
using UtyMap.Unity.Maps.Data;
using UtyRx;
using IContainer = UtyDepend.IContainer;
using Container = UtyDepend.Container;
using Component = UtyDepend.Component;

namespace Assets.Scripts
{
    class ApplicationManager
    {
        private const string FatalCategoryName = "Fatal";

        private IContainer _container;
        private DebugConsoleTrace _trace;
        private CompositionRoot _compositionRoot;
        private ITileController _tileController;

        private int _zoomLevel = 16;

        #region Singleton implementation

        private ApplicationManager()
        {
        }

        public static ApplicationManager Instance { get { return Nested.__instance; } }

        private class Nested
        {
            // Explicit static constructor to tell C# compiler
            // not to mark type as beforefieldinit
            static Nested() { }

            internal static readonly ApplicationManager __instance = new ApplicationManager();
        }

        #endregion

        #region Initialization logic

        public void InitializeFramework(ConfigBuilder configBuilder, Action<CompositionRoot> initAction)
        {
            // create default container which should not be exposed outside
            // to avoid Service Locator pattern.
            _container = new Container();

            // create trace to log important messages
            _trace = new DebugConsoleTrace();

            // subscribe to unhandled exceptions in RX
            MainThreadDispatcher.RegisterUnhandledExceptionCallback(ex =>
                _trace.Error(FatalCategoryName, ex, "Unhandled exception"));

            // console is way to debug/investigate app behavior on real devices when
            // regular debugger is not applicable
            CreateConsole();

            try
            {
                // create entry point for utymap functionallity
                _compositionRoot = new CompositionRoot(_container, configBuilder.Build())
                    .RegisterAction((c, _) => c.RegisterInstance<ITrace>(_trace))
                    .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<DemoPathResolver>()))
                    .RegisterAction((c, _) => c.Register(Component.For<IModelBuilder>().Use<DemoModelBuilder>()))
                    .RegisterAction((c, _) => c.Register(Component.For<CustomizationService>().Use<CustomizationService>()))
                    .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(@"/MapCss/default/default.mapcss")));

                // this is the way to insert custom extensions from outside. You may need to do it for
                // some scenes.
                initAction(_compositionRoot);

                // setup object graph
                _compositionRoot.Setup();
            }
            catch (Exception ex)
            {
                _trace.Error(FatalCategoryName, ex, "Cannot setup object graph.");
                throw;
            }
        }

        /// <summary> Creates debug console in scene. </summary>
        private void CreateConsole()
        {
            // NOTE DebugConsole is based on some adapted solution found in Internet
            var consoleGameObject = new GameObject("_DebugConsole_");
            var console = consoleGameObject.AddComponent<DebugConsole>();
            _trace.SetConsole(console);
            // that is not nice, but we need to use commands registered in DI with their dependencies
            console.SetContainer(_container);
            console.IsOpen = true;
        }

        #endregion

        #region Service locator

        /// <summary> Gets service of T from container. </summary>
        public T GetService<T>()
        {
            return _container.Resolve<T>();
        }

        /// <summary> Gets services of T from container. sS</summary>
        public IEnumerable<T> GetServices<T>()
        {
            return _container.ResolveAll<T>();
        }

        #endregion

        #region Public members

        public bool IsInitialized { get; private set; }

        public void RunGame()
        {
            try
            {
                if (IsInitialized)
                    throw new InvalidOperationException("Should not call RunGame more than once.");

                _tileController = GetService<ITileController>();

                GetService<IMessageBus>()
                    .AsObservable<TileLoadStartMessage>()
                    .ObserveOn(Scheduler.MainThread)
                    .Subscribe(m => m.Tile.GameObject = new GameObject("tile"));

                // NOTE this is just example: you can load your regions into memory once
                // game is started. Also you can specify different zoom level if you
                // have valid mapcss stylesheet
                _compositionRoot.GetService<IMapDataLoader>()
                    .AddToInMemoryStore(@"Osm/berlin.osm.xml", 
                    _compositionRoot.GetService<Stylesheet>(), 
                    new Range<int>(_zoomLevel, _zoomLevel));

                IsInitialized = true;
            }
            catch (Exception ex)
            {
                _trace.Error("FATAL", ex, "Error running game:");
                throw;
            }
        }

        /// <summary> Notifies utymap about position change. </summary>
        public void SetPosition(Vector2 point)
        {
            Scheduler.ThreadPool.Schedule(() => _tileController.OnPosition(point, _zoomLevel));
        }

        /// <summary> Notifies utymap about coordinate change. </summary>
        public void SetPosition(GeoCoordinate coordinate)
        {
            Scheduler.ThreadPool.Schedule(() => _tileController.OnPosition(coordinate, _zoomLevel));
        }

        /// <summary> Sets zoom level. </summary>
        public void SetZoomLevel(int zoomLevel)
        {
            _zoomLevel = zoomLevel;
        }

        #endregion
    }
}
