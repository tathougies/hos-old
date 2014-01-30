{-# LANGUAGE NoImplicitPrelude, MagicHash #-}
module Main where

	import GHC.Types
        import GHC.Prim

        ($) :: (a -> b) -> a -> b
        f $ x = f x

        unIO :: IO a -> (State# RealWorld -> (# State# RealWorld, a #))
        unIO (IO a) = a


        returnIO :: a -> IO a
        returnIO x = IO $ \ s -> (# s, x #)

        bindIO :: IO a -> (a -> IO b) -> IO b
        bindIO (IO m) k = IO $ \ s -> case m s of (# new_s, a #) -> unIO (k a) new_s

	main :: IO ()
	main = returnIO ()